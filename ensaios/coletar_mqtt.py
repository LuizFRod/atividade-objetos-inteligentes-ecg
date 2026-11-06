"""Coleta MQTT 3.1.1 QoS0 para o demonstrador sintetico, sem dependencias externas."""
import argparse, datetime, json, socket, struct, time, uuid
from pathlib import Path

def field(text):
    raw=text.encode(); return struct.pack('!H',len(raw))+raw

def packet(header, body):
    size=len(body); rem=bytearray()
    while True:
        value=size % 128; size //=128; rem.append(value | (128 if size else 0))
        if not size: break
    return bytes([header])+rem+body

def read_exact(stream,count):
    data=bytearray()
    while len(data)<count:
        part=stream.recv(count-len(data))
        if not part: raise ConnectionError('Broker encerrou conexao')
        data.extend(part)
    return bytes(data)

def receive(stream):
    header=read_exact(stream,1)[0]; size=0; scale=1
    for _ in range(4):
        value=read_exact(stream,1)[0];size+=(value&127)*scale;scale*=128
        if not value&128: break
    else: raise ValueError('Comprimento MQTT invalido')
    return header,read_exact(stream,size)

def main():
    cli=argparse.ArgumentParser()
    cli.add_argument('--segundos',type=int,default=600)
    cli.add_argument('--saida',default='dados/ensaio-600s.json')
    args=cli.parse_args()
    if args.segundos<1:raise ValueError('Duracao deve ser positiva')
    started=time.monotonic();wall=time.time(); records=[]; events=[]
    target=Path(args.saida);target.parent.mkdir(parents=True,exist_ok=True)
    stream=socket.create_connection(('broker.hivemq.com',1883),timeout=15)
    with stream:
        stream.settimeout(30)
        stream.sendall(packet(0x10,field('MQTT')+bytes([4,2])+struct.pack('!H',30)+field('ecg-coleta-'+uuid.uuid4().hex[:10])))
        header,body=receive(stream)
        if header!=0x20 or len(body)<2 or body[1]!=0:raise ConnectionError('CONNACK recusado')
        stream.sendall(packet(0x82,b'\0\1'+field('aula/ecg/esp32-84729/amostras')+b'\0'))
        stream.settimeout(2)
        print('Conectado ao broker. Coleta iniciada.',flush=True)
        ping=time.monotonic()
        while time.monotonic()-started<args.segundos:
            if time.monotonic()-ping>15:
                stream.sendall(packet(0xC0,b''));ping=time.monotonic()
            try:header,body=receive(stream)
            except socket.timeout:continue
            if header>>4!=3:continue
            size=struct.unpack('!H',body[:2])[0]
            try:payload=json.loads(body[2+size:])
            except (ValueError,UnicodeDecodeError):continue
            if payload.get('origem')!='ad8232-simulado':continue
            records.append({'recebido_unix_ms':round(time.time()*1000),'topico':body[2:2+size].decode(),'payload':payload})
        stream.sendall(packet(0xE0,b''))
    duration=time.monotonic()-started
    samples=sum(len(r['payload']['amostras']) for r in records)
    resets=sum(records[i]['payload']['indice']<records[i-1]['payload']['indice'] for i in range(1,len(records)))
    delta=[records[i]['payload']['indice']-records[i-1]['payload']['indice']-len(records[i-1]['payload']['amostras']) for i in range(1,len(records))]
    positive=sum(x for x in delta if x>0)
    report={'inicio_utc':datetime.datetime.fromtimestamp(wall,datetime.timezone.utc).isoformat(),'duracao_receptor_s':round(duration,3),'lotes_recebidos':len(records),'amostras_recebidas':samples,'regressoes_indice':resets,'lacunas_indices_observadas':positive,'nota':'Coleta no receptor por tempo de relogio. Indices e frequencia sao do tempo simulado. Lacunas incluem efeitos de aquisicao e transporte; nao medem taxa de entrega isoladamente. Mais de uma instancia publicadora pode invalidar comparacoes.','mensagens':records,'eventos':events}
    target.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k not in ('mensagens','eventos')},ensure_ascii=False))

if __name__=='__main__':main()
