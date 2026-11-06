# ECG sintético com ESP32 e AD8232 simulado

Projeto acadêmico de Luiz Felipe Rodrigues, Universidade Presbiteriana Mackenzie. Um componente externo gera a saída analógica sintética, o ESP32 lê o ADC, atua no LED e publica por MQTT TLS autenticado. O Node-RED apresenta a curva e os estados.

- [Simulação atual com TLS](https://wokwi.com/projects/479900142449265665)
- [Artigo em PDF](documentacao/artigo-ecg-revisado.pdf)
- [Artigo editável](documentacao/artigo-ecg-revisado.docx)
- [Instalação e configuração](documentacao/INSTALACAO.md)
- [Segurança e testes](seguranca/ATIVACAO-TLS.md)
- **YouTube não listado: o autor ainda precisa gravar e inserir o link no artigo.**

O AD8232 SIMULADO é um modelo funcional de interface com sinal sintético. Não reproduz toda a eletrônica do AD8232, não utiliza pessoas e não realiza diagnóstico. A aceitação como sensor simulado depende do professor.

## Executar

1. Abra a simulação atual, inicie e aguarde o pedido de senha no monitor serial.
2. Digite ali a senha do usuário `esp32-84729` e pressione Enter. Ela fica somente em RAM, não é impressa e precisa ser informada novamente após reiniciar. Nunca a grave no código público ou no GitHub.
3. No Node-RED 5, instale `@flowfuse/node-red-dashboard`. Importe `node-red/fluxo-ecg.json` em nova aba somente se ela ainda não existir.
4. Configure **ECG seguro • EMQX** com usuário `painel-ecg`; configure **ECG • teste publicador TLS** com usuário `esp32-84729`. Informe as senhas apenas no editor local. Nas configurações TLS, mantenha a validação do servidor e carregue `seguranca/emqxsl-ca.crt` como CA.
5. Faça Deploy e abra `http://localhost:1880/dashboard-ecg-84729/ad8232`. O Node-RED deve permanecer em execução. Em outra instalação, confira o caminho base do Dashboard.
6. Chave no GND: contato normal. Chave em 3V3: LED aceso e gráfico limpo/suspenso. Retorne ao GND para retomar.
7. Para o teste manual, pause o Wokwi e pressione o inject de 250 amostras. O lote percorre MQTT OUT → broker TLS → MQTT IN → gráfico. Não misture esse teste com o ensaio do dispositivo.

A exportação não inclui senhas nem o conteúdo de credenciais do Node-RED. Quem reproduzir deve usar a própria implantação e credenciais autorizadas. O endereço e a CA são públicos; o certificado CA não é chave privada.

## Comunicação

Broker EMQX Cloud Serverless: `i066e913.ala.us-east-1.emqxsl.com:8883`. MQTT 3.1.1, TLS com validação de CA/hostname, usuário e senha; tópico `aula/ecg/esp32-84729/amostras`, QoS 0 e retain=false. A infraestrutura é compartilhada pelo provedor, com acesso do projeto restrito por autenticação e permissões; não é um servidor dedicado.

O firmware usa `WiFiClientSecure`, aguarda horário NTP, solicita a senha pelo serial e não oferece alternativa sem criptografia. O payload mantém dispositivo, indice, intervalo_us, eletrodos, origem, perdidas_temporizacao e amostras; acrescenta transporte=`mqtts` e sessao para identificar uma execução.

A política de tópicos deve permitir somente publicação a `esp32-84729` e assinatura a `painel-ecg` no tópico acima, com negação geral de demais operações. Veja as regras e os limites da validação em `seguranca/ATIVACAO-TLS.md`.

## Evidências

- Firmware seguro compilado e executado no Wokwi; autenticação aceita e lotes recebidos no Node-RED.
- Coleta TLS em janela de 60 s: 372 lotes e 9300 valores. Entre o primeiro e o último lote: 59.834 s. Uma sessão; 0 regressões e 352 posições ausentes entre lotes. JSONL, CSV e resumo em `dados/`.
- Teste manual de 250 valores retornou pelo broker seguro. Contato/desconexão/reconexão e limpeza explícita do gráfico foram verificados na versão TLS.
- Acesso anônimo e senha incorreta recusados (CONNACK 4). Hostname divergente rejeitado pelo cliente TLS. Resultados em `ensaios/seguranca-tls.json`; os testes não revelam senhas.
- Os ensaios anteriores `ensaio-curto.json` e `ensaio-600s.json.gz` foram feitos no HiveMQ público sem TLS e ficam preservados como histórico. Não são evidência de transporte seguro.

O gateway público do Wokwi encerrou uma conexão durante o ensaio. A simulação foi reiniciada e autenticada novamente. Não foi comprovada disponibilidade contínua ou prazo máximo de recuperação.

## Aquisição e limites

GPIO34 lê o OUTPUT do modelo; GPIO32/33 recebem LOP/LON. LED GPIO2 com resistor 220 Ω: desconexão tem prioridade; sem MQTT pisca a cada 500 ms; em operação normal indica ADC acima do limiar didático 2000.

São 25 leituras por lote, intervalo nominal 4000 µs (250 Hz). Rede e aquisição compartilham o loop e podem perder períodos. Lacunas de índices não isolam perda de rede, e o horário do gráfico é reconstruído no receptor. Não se demonstraram 99% de entrega, latência abaixo de 1 s, recuperação em 30 s, calibração para mV ou uso clínico. Não há comando MQTT remoto do LED nem buffer circular persistente.

## Estrutura

`wokwi/`: firmware e circuito; `node-red/`: fluxo importável; `dados/`: dados sintéticos capturados; `ensaios/`: testes e coletores; `seguranca/`: CA pública e configuração; `imagens/`: diagramas e capturas; `documentacao/`: artigo e instruções.

Os exemplos Mosquitto permanecem como alternativa de hospedagem, não como serviço utilizado neste ensaio. Fotografias de referência física foram recuperadas do artigo-base com atribuição a Espressif e SparkFun; não comprovam montagem física.
