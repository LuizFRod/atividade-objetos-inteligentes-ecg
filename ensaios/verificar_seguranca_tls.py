import socket, ssl, json, struct, datetime, time
from pathlib import Path
HOST='i066e913.ala.us-east-1.emqxsl.com'; PORT=8883
ROOT=Path(__file__).resolve().parents[1]
CA=ROOT/'seguranca/emqxsl-ca.crt'
report={'data_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'broker':HOST,'porta':PORT,'testes':[]}
def field(s):
 b=s.encode(); return struct.pack('!H',len(b))+b

def probe(name, user=None, password=None):
 ctx=ssl.create_default_context(cafile=str(CA))
 with socket.create_connection((HOST,PORT),timeout=30) as tcp:
  with ctx.wrap_socket(tcp,server_hostname=HOST) as tls:
   flags=2 | (0x80 if user is not None else 0) | (0x40 if password is not None else 0)
   payload=field('verificacao-ecg-'+str(time.time_ns()))
   if user is not None:payload+=field(user)
   if password is not None:payload+=field(password)
   body=field('MQTT')+bytes([4,flags])+struct.pack('!H',30)+payload
   left=len(body); enc=bytearray()
   while True:
    digit=left%128;left//=128;enc.append(digit|(128 if left else 0))
    if not left:break
   tls.sendall(bytes([0x10])+enc+body)
   response=b''
   while len(response)<4:
    chunk=tls.recv(4-len(response))
    if not chunk:break
    response+=chunk
   code=response[3] if len(response)==4 and response[:2]==b'\x20\x02' else None
   report['testes'].append({'nome':name,'tls':tls.version(),'hostname_validado':True,'cadeia_CA_validada':True,'connack':code,'resultado':'PASSOU' if code in (4,5) else 'NAO_CONFIRMADO'})
probe('MQTT anonimo recusado')
probe('Senha propositalmente incorreta recusada','painel-ecg','senha-incorreta-de-teste-nao-e-credencial')
try:
 ctx=ssl.create_default_context(cafile=str(CA))
 with socket.create_connection((HOST,PORT),timeout=30) as tcp:
  with ctx.wrap_socket(tcp,server_hostname='hostname-incorreto.example'):pass
 report['testes'].append({'nome':'Nome de servidor divergente','resultado':'FALHOU'})
except ssl.SSLCertVerificationError:
 report['testes'].append({'nome':'Nome de servidor divergente','resultado':'PASSOU','observacao':'O cliente rejeitou certificado cujo nome nao corresponde ao hostname; validacao nao foi desabilitada.'})
p=ROOT/'ensaios/seguranca-tls.json';p.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
print(json.dumps(report,ensure_ascii=False,indent=2))
