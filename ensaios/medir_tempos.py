import argparse,json,time,uuid,statistics,csv
from pathlib import Path
from urllib.request import Request,urlopen

def main():
 ap=argparse.ArgumentParser(description="Mede tempos de resposta MQTT do sensor e do LED")
 ap.add_argument('--url',default='http://127.0.0.1:1880')
 ap.add_argument('--saida',default='dados/tempos-resposta')
 args=ap.parse_args();out=Path(args.saida);out.mkdir(parents=True,exist_ok=True)
 raw=[];rows=[];prefix=uuid.uuid4().hex[:8];counter=0
 def request(path,data=None):
  req=Request(args.url+path,data=json.dumps(data).encode() if data is not None else None,headers={'Content-Type':'application/json'})
  with urlopen(req,timeout=5) as r:return json.load(r)
 def command(op,valor=0):
  nonlocal counter
  counter+=1;id=f'{prefix}_{counter}'
  request('/ecg-ensaio/send',dict(op=op,id=id,valor=valor))
  deadline=time.monotonic()+15
  while time.monotonic()<deadline:
   found=[x for x in request('/ecg-ensaio/resultados') if x['id']==id]
   if found:
    e=found[0];raw.append(e);(out/'eventos.json').write_text(json.dumps(raw,ensure_ascii=False,indent=2))
    if e['tipo']=='recusado_contato':raise RuntimeError('Chave de eletrodos desconectada. Restabeleca o contato.')
    return e
   time.sleep(.08)
  raise TimeoutError('ESP32 nao respondeu. Confira execucao, senha e Subscribe/Allow no topico exato.')
 try:

  command('LED',1)
  for op in ('SENSOR','LED'):
   for i in range(1,5):
    if op=='LED':command('LED',1 if i%2 else 0)
    a=command('CLOCK');c=command('CLOCK');e=command(op,0 if i%2 else 1);b=command('CLOCK')
    if len({a['sessao'],c['sessao'],e['sessao'],b['sessao']})!=1:raise RuntimeError('Sessao mudou durante a medida.')
    ea,eb=a['evento_us']/1000,b['evento_us']/1000
    if not ea<e['evento_us']/1000<eb:raise RuntimeError('Relogio nao monotono.')
    ha=(a['enviado_ms']+a['recebido_ms'])/2;hb=(b['enviado_ms']+b['recebido_ms'])/2
    scale=(hb-ha)/(eb-ea);alpha=(e['evento_us']/1000-ea)/(eb-ea)
    mapped=ha+alpha*(hb-ha)
    rtta=a['recebido_ms']-a['enviado_ms'];rttb=b['recebido_ms']-b['enviado_ms']
    residual=abs((c['enviado_ms']+c['recebido_ms'])/2-(ha+(c['evento_us']/1000-ea)*scale))
    uncertainty=((1-alpha)*rtta+alpha*rttb)/2+residual+1
    rtt=e['recebido_ms']-e['enviado_ms']
    latency=e['recebido_ms']-mapped if op=='SENSOR' else mapped-e['enviado_ms']
    if latency < -uncertainty or latency > rtt+uncertainty:raise RuntimeError('Conversao de relogio inconsistente; nao publicar media.')

    lo=max(0,latency-uncertainty);hi=min(rtt,latency+uncertainty)
    if hi<lo:raise RuntimeError('Intervalo de incerteza invalido.')
    rows.append(dict(medida=i,elemento='Sensor ADC' if op=='SENSOR' else 'Atuador LED',tempo_estimado_ms=round((lo+hi)/2,3),estimativa_afim_ms=round(latency,3),residuo_relogio_ms=round(residual,3),limite_inferior_ms=round(lo,3),limite_superior_ms=round(hi,3),incerteza_relogio_ms=round(uncertainty,3),ida_volta_ms=rtt,escala_relogio=round(scale,6),id=e['id'],valor=e['valor']))
    print(f'{op} {i}: {latency:.1f} ms; intervalo [{lo:.1f}, {hi:.1f}] ms; RTT {rtt} ms',flush=True)
  with (out/'medidas.csv').open('w',newline='') as f:
   w=csv.DictWriter(f,fieldnames=rows[0].keys());w.writeheader();w.writerows(rows)
  summary={k:{'n':4,'media_estimada_ms':statistics.mean(r['tempo_estimado_ms'] for r in rows if r['elemento']==k),'media_limite_inferior_ms':statistics.mean(r['limite_inferior_ms'] for r in rows if r['elemento']==k),'media_limite_superior_ms':statistics.mean(r['limite_superior_ms'] for r in rows if r['elemento']==k)} for k in ('Sensor ADC','Atuador LED')}
  (out/'resumo.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2));print(json.dumps(summary,ensure_ascii=False),flush=True)
 finally:
  try:command('AUTO')
  except Exception:pass
if __name__=='__main__':main()
