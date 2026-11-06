# Dados sintéticos capturados

## Versão atual com TLS

`ensaio-tls-60s.jsonl` contém 372 lotes recebidos pelo MQTT IN autenticado do Node-RED. `amostras-ensaio-tls.csv` contém os 9300 valores individuais; `resumo-ensaio-tls.json` contém as estatísticas. A janela de coleta foi configurada em 60 s, e o intervalo entre o primeiro e o último lote é 59,834 s. Todos os lotes pertencem à mesma sessão. Foram observadas 352 posições ausentes entre lotes e nenhuma regressão. Lacunas não isolam perda de rede.

O marcador transporte=mqtts identifica a versão do firmware; a comprovação do transporte vem também da configuração TLS do receptor, do handshake e do log de execução. Um campo JSON isolado não prova criptografia.

## Ensaios históricos sem TLS

`ensaio-curto.json` tem dez lotes e 250 valores. `amostras-ensaio-curto.csv` é a tabela correspondente. `ensaio-600s.json.gz` contém os dados completos históricos (também disponíveis descomprimidos no pacote local) e `resumo-ensaio-600s.json` as estatísticas: 600,346 s, 1682 lotes, 42050 valores, 1143 posições ausentes e nenhuma regressão. Esses ensaios utilizaram broker.hivemq.com:1883 sem TLS; não comprovam a segurança da versão atual.

A frequência 250 Hz é nominal no tempo do simulador. O horário de chegada no computador não equivale ao instante de aquisição. Nenhum arquivo contém ECG de pessoas ou credenciais reais.
