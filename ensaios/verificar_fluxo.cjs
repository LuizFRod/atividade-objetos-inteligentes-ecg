const fs=require('fs'),assert=require('assert/strict');
const f=JSON.parse(fs.readFileSync('node-red/fluxo-ecg.json'));

const ids=new Set(f.map(n=>n.id));assert.equal(ids.size,f.length);
for(const n of f){
 for(const k of ['z','group','ui','page','theme'])if(n[k])assert(ids.has(n[k]));
 if(n.broker&&n.type!=='mqtt-broker')assert(ids.has(n.broker));
 for(const w of n.wires||[])for(const id of w)assert(ids.has(id));
 if(n.type==='function')new Function('msg','node','context','flow',n.func);
}
const parser=f.find(n=>n.type==='function'&&n.outputs===2);
const c=new Map(),g=new Map(); const store=m=>({get:k=>m.get(k),set:(k,v)=>m.set(k,v)});
const run=new Function('msg','node','context','flow',parser.func);let errors=[];
const invoke=p=>run({payload:p},{error:e=>errors.push(e),status:()=>{}},store(c),store(g));
const batch={dispositivo:'esp32-84729',indice:25,intervalo_us:4000,eletrodos:'conectados',origem:'ad8232-simulado',amostras:[1400,1700,2200,1400]};
let r=invoke(JSON.stringify(batch));assert.equal(r[0].payload.length,4);assert.equal(r[0].payload[1].x-r[0].payload[0].x,4);
r=invoke({...batch,eletrodos:'desconectados'});assert.deepEqual(r[0].payload,[]);assert.match(r[1].payload,/suspenso/);
r=invoke({...batch,eletrodos:'desconectados'});assert.equal(r[0],null);
r=invoke(batch);assert.equal(r[0].payload.length,4);
assert.equal(invoke({...batch,amostras:[]}),null);assert.equal(invoke({...batch,eletrodos:'talvez'}),null);
assert.equal(errors.length,2);
const d=JSON.parse(fs.readFileSync('wokwi/diagram.json'));
const chip=JSON.parse(fs.readFileSync('wokwi/ad8232.chip.json'));
for(const conn of d.connections)for(const endpoint of conn.slice(0,2)){
 const [id,pin]=endpoint.split(':');assert(d.parts.some(p=>p.id===id));
 if(id==='ecg')assert(chip.pins.includes(pin));
}
const firmware=fs.readFileSync('wokwi/sketch.ino','utf8');assert(firmware.includes('analogRead(ECG)'));assert(!firmware.includes('gerarECG('));
console.log('PASS: JSON/IDs/referências, GPIO34 ADC, lote conectado, desconexão limpa gráfico, reconexão, casos inválidos e pinagem custom chip.');
