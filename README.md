# ECG sintético com ESP32 e AD8232 simulado

Projeto acadêmico de Luiz Felipe Rodrigues, Universidade Presbiteriana Mackenzie. Demonstra geração externa de sinal sintético, leitura pelo ADC do ESP32, MQTT pela Internet, sinalização por LED e visualização no Node-RED.

- [Executar no Wokwi](https://wokwi.com/projects/479896326597353473)
- [Repositório](https://github.com/LuizFRod/atividade-objetos-inteligentes-ecg)
- [Texto do artigo](documentacao/artigo-texto.md)
- **Vídeo YouTube não listado: pendente de gravação e inclusão pelo autor.**

O componente AD8232 SIMULADO é um modelo funcional didático com saída sintética. Não reproduz o circuito interno do AD8232, não utiliza eletrodos humanos e não realiza diagnóstico. A aceitação como sensor simulado na disciplina depende do docente.

## Estrutura

| Pasta | Conteúdo |
|---|---|
| wokwi/ | sketch.ino, diagram.json, modelo .chip.c/.chip.json e bibliotecas |
| node-red/ | fluxo JSON com uma nova aba, sem substituir as existentes |
| dados/ | mensagens sintéticas realmente capturadas do broker |
| ensaios/ | coletor MQTT e verificações locais |
| imagens/ | esquemas, fluxograma e capturas de execução |
| documentacao/ | artigo, instalação, roteiro do vídeo e limites |
| seguranca/ | modelos de implantação TLS ainda não ativados |

## Executar

1. Abra o projeto Wokwi e pressione iniciar. Para reconstruir em um projeto novo, siga [a instalação](documentacao/INSTALACAO.md).
2. No Node-RED, instale **@flowfuse/node-red-dashboard** pelo gerenciador de paleta.
3. Importe `node-red/fluxo-ecg.json` em **nova aba** e faça Deploy. Não importe novamente se a aba já existir.
4. Abra `http://localhost:1880/dashboard-ecg-84729/ad8232`. O Node-RED precisa permanecer rodando. Se o Dashboard existente utiliza outro caminho base, ajuste o endereço conforme a barra lateral Dashboard 2.0.
5. Com a chave no GND, o estado é conectado. Na posição ligada a 3V3, o modelo indica desconexão: LED aceso e gráfico suspenso. Retorne a chave para retomar o sinal.

Pare outras instâncias que publiquem no mesmo tópico durante um teste isolado. O inject manual também usa esse tópico: teste-o separadamente.

## Comunicação e aquisição

Broker público `broker.hivemq.com:1883`, sem TLS/autenticação; tópico `aula/ecg/esp32-84729/amostras`, QoS 0, retain=false. Payload: dispositivo, indice, intervalo_us, eletrodos, origem, perdidas_temporizacao e amostras. O índice identifica a primeira amostra do lote. Cada lote possui 25 leituras; intervalo nominal 4000 µs, correspondente a 250 Hz.

O módulo externo gera a onda a cada 1 ms, com 75 BPM inicialmente. O ESP32 usa `analogRead(34)`. A rede e a aquisição compartilham a rotina; atrasos podem causar perda de períodos. O contador não mede perda total de rede. O eixo do gráfico usa horário reconstruído no receptor, sem sincronização do instante de aquisição.

O LED GPIO2 usa resistor de 220 Ω: desconexão tem prioridade e mantém o LED aceso; MQTT indisponível produz alternância de 500 ms; com contato e MQTT normais, o LED acende quando o ADC supera o limiar didático 2000. Não há comando MQTT remoto do LED.

## Evidências e limites

Foram testados compilação, leitura ADC, recepção MQTT, gráfico e desconexão/reconexão de contato. As mensagens em `dados/` são de simulação, não de pacientes. A coleta curta contém dez lotes, totalizando 250 valores. O arquivo do ensaio estendido registra seu próprio tempo, contagem e observações.

A taxa de 250 Hz é configurada, não comprovada como aquisição contínua sem perdas. Não se demonstraram 99% de entrega, latência inferior a 1 s ou reconexão em 30 s. A simulação analógica é simplificada, sem calibração para mV de ECG.

O broker público não atende à segurança prevista na proposta física. A pasta `seguranca/` prepara uma implantação com certificado, autenticação e ACL; não indica que ela já foi ativada. Não publique secrets.h, certificados privados, senhas ou credenciais de rede.

O artigo deve receber o link do vídeo antes da submissão. Também é necessário confirmar a aceitação do modelo simulado e conferir os enunciados anteriores da disciplina, não fornecidos integralmente nesta revisão.

## Fontes e autoria das imagens

Firmware, modelo, diagramas e documentação são materiais do projeto acadêmico. As fotografias de referência física e ilustrações históricas foram recuperadas do artigo-base; as fotografias mantêm atribuição a Espressif Systems e SparkFun. Não se atribui licença a material de terceiros.
