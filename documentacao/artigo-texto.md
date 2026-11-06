UNIVERSIDADE PRESBITERIANA MACKENZIE

Faculdade de Computação e Informática
Objetos Inteligentes Conectados

Luiz Felipe Rodrigues
Universidade Presbiteriana Mackenzie
São Paulo SP Brasil

Resumo

Este trabalho apresenta um demonstrador acadêmico de Internet das Coisas para visualizar um sinal ECG sintético. A implementação utiliza ESP32 no Wokwi, um modelo funcional de interface do AD8232, LED externo e comunicação MQTT pela Internet. O componente personalizado gera uma saída analógica sintética, lida pelo ADC do ESP32 em lotes de 25 valores, com intervalo nominal de 4000 microssegundos. Uma chave representa a condição dos eletrodos; o LED indica desconexão, indisponibilidade MQTT e picos sintéticos. No Node-RED, o FlowFuse Dashboard exibe a curva e os estados de operação. A execução demonstrou publicação, recepção, visualização e suspensão do traçado durante desconexão simulada. O transporte utilizado no ensaio foi HiveMQ público, porta 1883, sem TLS ou autenticação. O modelo não reproduz a eletrônica interna do AD8232 nem adquire sinais de pessoas. A contribuição ao ODS 3 é educacional, sem diagnóstico ou comprovação de benefício clínico.

Palavras chave: Internet das Coisas; ECG sintético; ESP32; AD8232 simulado; MQTT; ODS 3.

Abstract

This paper presents an educational Internet of Things demonstrator for visualizing a synthetic ECG waveform. The implementation uses an ESP32 in Wokwi, a functional AD8232 interface model, an external LED, and Internet MQTT communication. The custom component generates an analog output that is sampled by the ESP32 ADC and transmitted in batches of 25 values at a nominal interval of 4000 microseconds. A switch represents electrode contact, while the LED indicates disconnection, MQTT availability, and synthetic peaks. A Node-RED FlowFuse Dashboard displays the waveform and device states. Functional tests demonstrated publishing, reception, visualization, and waveform suspension during simulated disconnection. The experiment used the public HiveMQ broker on port 1883 without TLS or authentication. The model does not reproduce the internal AD8232 electronics or acquire human signals. Its contribution to SDG 3 is educational, without diagnostic claims or evidence of clinical benefit.

Keywords: Internet of Things; synthetic ECG; ESP32; simulated AD8232; MQTT; SDG 3.

1 Introdução

A integração entre entradas analógicas, processamento embarcado e comunicação pela Internet permite observar como sistemas de monitoramento disponibilizam dados a aplicações remotas. O projeto parte da proposta de monitoramento cardíaco com ESP32 e AD8232 desenvolvida nas etapas anteriores da disciplina (RODRIGUES, 2026). Nesta etapa, a execução foi delimitada a uma demonstração simulada, preservando o estudo da aquisição por ADC, organização de amostras, publicação MQTT e atuação visual.

O AD8232 físico é um circuito de condicionamento de biopotenciais, com recursos de amplificação, filtragem e detecção de desconexão de eletrodos (ANALOG DEVICES, s.d.). O componente criado neste trabalho representa apenas sua interface funcional de saída e estado. O sinal de teste é produzido matematicamente no próprio componente simulado, e o ESP32 lê sua saída. Essa distinção impede interpretar o gráfico como registro cardíaco real.

1.1 Objetivos e relação com o ODS 3

O objetivo geral é demonstrar a cadeia módulo de ECG simulado, ADC do ESP32, MQTT pela Internet, visualização e sinalização por LED. Os objetivos específicos são realizar a leitura de uma saída analógica externa ao firmware, transmitir lotes JSON, apresentar a curva no navegador, representar a perda de contato e documentar o código e os dados de ensaio.

O trabalho se relaciona tematicamente ao ODS 3 Saúde e Bem Estar, especialmente à meta 3.4 sobre doenças não transmissíveis (NAÇÕES UNIDAS BRASIL, s.d.). A contribuição é a aprendizagem de sistemas conectados aplicáveis ao estudo de monitoramento em saúde. Não foram avaliados pacientes, classificados eventos cardíacos ou medidos efeitos sobre mortalidade e bem estar.

1.2 Delimitação da implementação

A versão executada não utiliza AD8232 físico, eletrodos humanos ou montagem em protoboard. A proposta física anterior é mantida como possibilidade de extensão e documentada no Apêndice A. As seções seguintes descrevem a simulação efetivamente realizada. A aceitação do modelo funcional como sensor simulado para avaliação depende da orientação da disciplina.

Figura 1. Arquitetura da demonstração simulada e caminho das mensagens. A comunicação com o broker ocorre pela Internet; o Dashboard é servido pelo Node-RED local. Fonte: elaboração própria, 2026.

2 Materiais e Métodos

2.1 Plataforma e ambiente de desenvolvimento

Foi utilizada a placa ESP32 DevKitC V4 disponível no Wokwi. O Wi-Fi integrado permite ao firmware publicar diretamente no broker MQTT, dispensando Arduino Uno, Firmata ou uma ponte serial USB. O monitor serial é usado para depuração e não converte mensagens para MQTT. A leitura do GPIO34 utiliza analogRead(), cujo retorno corresponde ao valor bruto do ADC, sem calibração em milivolts (ESPRESSIF SYSTEMS, s.d. a).

O firmware foi escrito em C++ com Arduino ESP32, WiFi.h e PubSubClient. O componente externo foi escrito em C por meio da Custom Chips API do Wokwi, que permite definir pinos, temporizadores e controles de modelos personalizados (WOKWI, s.d. a). A aplicação receptora utiliza Node-RED 5.0.9 e o pacote @flowfuse/node-red-dashboard. Os códigos, arquivos de montagem e instruções de reprodução constam no repositório indicado na seção 3.4.

Elemento | Quantidade | Papel na versão executada

ESP32 DevKitC V4 simulado | 1 | Leitura ADC, entradas digitais, LED e cliente MQTT

AD8232 SIMULADO | 1 | Modelo de interface com geração sintética e estados LOP e LON

LED vermelho e resistor de 220 ohms | 1 de cada | Atuação visual no GPIO2

Chave deslizante de contato | 1 | Controle da entrada didática DESC

Wokwi e bibliotecas Arduino | Software | Compilação e execução do firmware e modelo

Node-RED e FlowFuse Dashboard | Software | Recepção, validação e visualização

Computador e acesso à Internet | Apoio | Editor, receptor MQTT e navegador

2.2 Modelo do sensor e geração do sinal

O modelo externo foi denominado AD8232 SIMULADO para identificar sua finalidade e seu limite. Ele possui OUTPUT analógico, LOP e LON digitais, SDN de habilitação e DESC como entrada didática de desconexão. LOP e LON representam funcionalmente LO+ e LO- do módulo físico. DESC não é um terminal real do AD8232. Os terminais VCC e GND aparecem no esquema como referência de montagem; o código do modelo não valida alimentação.

Um temporizador do componente atualiza a saída a cada milissegundo. O período é definido pela frequência sintética, inicialmente 75 BPM, ajustável entre 30 e 180 BPM. A fase normalizada é usada para somar cinco parcelas gaussianas, representando P, Q, R, S e T. O sinal é somado a uma referência de 1,65 V e aplicado à saída pela função pin_dac_write(). Não são modelados impedâncias, contato com a pele, interferência fisiológica, rejeição de modo comum ou resposta elétrica dos filtros.

g(f) = 0,12 exp(-((f-0,18)/0,04)^2)
     - 0,15 exp(-((f-0,37)/0,012)^2)
     + 1,00 exp(-((f-0,40)/0,010)^2)
     - 0,25 exp(-((f-0,43)/0,015)^2)
     + 0,30 exp(-((f-0,68)/0,09)^2)
OUTPUT = 1,65 + g(f), com f entre 0 e 1

A chave ligada ao GND mantém DESC baixo e sinaliza contato normal. Ligada a 3V3, torna DESC alto; o modelo eleva LOP e LON e fixa OUTPUT em 1,65 V. SDN baixo zera OUTPUT. Essas regras são escolhas do modelo para testar a aplicação, não uma reprodução validada do AD8232. O simulador possui suporte analógico simplificado; as contagens do ADC não devem ser convertidas diretamente para amplitude cardíaca em milivolts (WOKWI, s.d. b).

2.3 Modelo de montagem e interligações

OUTPUT foi ligado ao GPIO34, e LOP e LON aos GPIO32 e GPIO33. SDN foi ligado a 3V3. O LED foi conectado ao GPIO2 por resistor de 220 ohms, com cátodo no GND. A chave seleciona GND ou 3V3 para DESC. O circuito efetivo está no arquivo diagram.json; o componente é definido em ad8232.chip.json e ad8232.chip.c. A Figura 2 identifica as conexões e distingue a entrada de teste dos pinos equivalentes ao módulo físico.

Figura 2. Esquema funcional das ligações do demonstrador Wokwi. Os blocos representam terminais nomeados, sem escala física. A entrada DESC é exclusiva da simulação. Fonte: elaboração própria, 2026.

A atualização substitui a descrição de LED no GPIO25 e resistor de 330 ohms da proposta anterior pela montagem executada no GPIO2 com 220 ohms. A amplificação e filtragem do componente físico permanecem como fundamentação e possibilidade de extensão, sem serem apresentadas como ensaio realizado.

2.4 Funcionamento do firmware e do atuador

O setup configura o LED, as entradas digitais, o ADC com resolução nominal de 12 bits e sua atenuação. Em seguida, inicia a conexão ao ponto de acesso Wokwi GUEST e define o servidor MQTT. O identificador do cliente combina parte do identificador do ESP32 e um valor aleatório, reduzindo conflitos entre reinicializações. O firmware registra no serial os resultados de publicação, o índice e o estado de contato.

O loop acompanha Wi-Fi e MQTT e tenta reconectar em intervalos de cinco segundos. A aquisição utiliza micros() e intervalo nominal de 4000 microssegundos. Quando o atraso ultrapassa períodos completos, o firmware avança o índice e incrementa perdidas_temporizacao. O lote incompleto é descartado para não representar leituras separadas por lacuna como se fossem contíguas. Uma mudança de contato também reinicia o lote. A cada 25 leituras, é construída a mensagem e realizada a publicação.

A publicação e a reconexão ocorrem na mesma rotina da aquisição e podem introduzir atrasos. Portanto, 250 Hz é a taxa configurada, não uma taxa contínua sem perdas comprovada. O contador de temporização não contabiliza todos os descartes nem separa perdas de aquisição, publicação e rede. Esta versão não implementa o buffer circular de 500 amostras ou tarefas concorrentes previstos originalmente.

Prioridade | Condição observada pelo firmware | Saída do LED

1 | LOP ou LON em nível alto | Aceso continuamente

2 | Contato normal e MQTT desconectado | Alternância a cada 500 ms

3 | Contato normal e MQTT conectado | Aceso quando ADC > 2000; apagado nos demais valores

O limiar 2000 foi adotado para indicar visualmente picos do sinal sintético nesta simulação. Não corresponde a limiar clínico, detecção validada de pico R ou cálculo de frequência cardíaca. Não há comando remoto MQTT para testar o LED nesta versão. A retomada após reconexão do contato ocorre no lote seguinte, sem a janela de estabilização de dois segundos proposta para um sensor físico.

Figura 3. Fluxograma do modelo externo, leitura pelo ESP32, atuação e processamento no receptor. A aquisição e a rotina de rede compartilham o loop. Fonte: elaboração própria, 2026.

2.5 Comunicação MQTT e processamento no Node RED

Foi utilizado MQTT 3.1.1 por Wi-Fi e Internet com broker.hivemq.com, porta TCP 1883, sem TLS e sem autenticação, exclusivamente para o ensaio com dados sintéticos. O tópico único de amostras é aula/ecg/esp32-84729/amostras. As publicações utilizam QoS 0 e retain desativado. Não foram implementados tópicos separados de estado e comando, Last Will ou confirmação de execução de comandos. O uso de publicação e assinatura segue a organização do protocolo MQTT (OASIS, 2014).

Campo JSON | Significado

dispositivo | Identificador esp32-84729

indice | Índice da primeira amostra do lote

intervalo_us | Intervalo nominal entre amostras, igual a 4000

eletrodos | conectados ou desconectados segundo a chave

origem | Identificação ad8232-simulado

perdidas_temporizacao | Contador de períodos perdidos na rotina de aquisição

amostras | Vetor de 25 contagens do ADC simulado

O nó MQTT IN entrega o payload ao Debug e à função de validação. A função verifica identificação, índice, intervalo e valores numéricos, convertendo as amostras em pontos x e y. O horário é reconstruído no receptor a partir da chegada do lote e do intervalo informado, com avanço crescente entre lotes. O índice não é utilizado para reordenar ou recuperar mensagens, e nem todas as lacunas entre lotes são representadas explicitamente no traçado.

O ui-chart recebe vetores de pontos no modo append, com janela configurada de dez segundos e limite adicional de 5000 pontos, conforme o mapeamento do FlowFuse Dashboard (FLOWFUSE, s.d.). Quando o estado é desconectados, o gráfico é limpo uma vez e deixa de receber os valores do lote. O painel mantém separados o último evento de conexão MQTT, a recepção recente e os metadados do contato. Uma verificação a cada dois segundos detecta ausência de mensagens pelo limite de dez segundos ou três vezes a duração nominal do último lote, prevalecendo o maior.

A nova aba foi importada sem substituir os fluxos anteriores. O painel local encontra-se no caminho /dashboard-ecg-84729/ad8232. O endereço localhost é restrito ao computador que executa o Node-RED; não representa hospedagem pública. Há também um teste manual inject para MQTT OUT, cuja mensagem só alimenta o gráfico após retornar pelo broker. Ele deve ser executado separadamente da simulação para não misturar as fontes (OPENJS FOUNDATION, s.d.).

2.5.1 Segurança da comunicação

A execução demonstrada não atende à conexão com TLS, autenticação individual e permissões de tópicos prevista na proposta anterior. O broker de teste é compartilhado e não fornece privacidade do tópico. A implantação de Mosquitto em servidor próprio ainda depende de infraestrutura e credenciais sob controle do autor; nenhum ensaio seguro é apresentado como concluído.

O repositório inclui modelos de configuração Mosquitto com listener TLS, usuários separados e ACL, além de uma variante de firmware com WiFiClientSecure, CA confiável e verificação de horário. A variante foi compilada no Wokwi com definições vazias de teste e bloqueou o MQTT por falta de configuração. Esses arquivos são preparação de implantação, não evidência de serviço ativo. Sua ativação e validação com certificado, DNS e credenciais reais permanecem pendentes. As configurações correspondem aos mecanismos documentados pelo Eclipse Mosquitto (ECLIPSE FOUNDATION, s.d.).

2.6 Procedimentos de verificação e registro

Foram verificadas a compilação e execução do firmware e do componente, a publicação no monitor serial, a recepção do JSON, a conversão dos lotes e a visualização no navegador. A chave foi acionada para provocar desconexão e posterior retorno, observando-se o LED e o tratamento do traçado. Foram preservadas capturas da montagem e das duas condições do painel.

O ensaio curto registrou dez mensagens realmente recebidas do broker, identificadas por origem ad8232-simulado, totalizando 250 valores. Os dados incluem horário de chegada no receptor, tópico e payload. A verificação local conferiu sintaxe e referências do fluxo, o espaçamento nominal de quatro milissegundos por ponto e a rejeição de campos inválidos. O teste não demonstra, por si só, taxa de entrega ou atraso de rede.

Também foi executada coleta por aproximadamente dez minutos de relógio do receptor, utilizando o script ensaios/coletar_mqtt.py. O programa assina o tópico e registra mensagens do modelo. A duração é medida por relógio monotônico do computador. O tempo do Wokwi pode avançar a velocidade diferente do tempo real; por isso, frequência nominal, amostras por segundo de chegada e cobertura de índices não são tratadas como medidas equivalentes.

3 Resultados

3.1 Integração e comportamento observado

A demonstração apresentou leituras variáveis do ADC GPIO34, publicação MQTT e gráfico com ciclos da forma de onda sintética. A Figura 4 mostra a execução conectada. Os metadados indicam dispositivo, índice da primeira amostra, quantidade e taxa nominal. A forma de onda resulta do modelo matemático externo, sem captação de ECG humano.

Figura 4. Dashboard durante recepção de lotes e apresentação de sinal sintético lido pelo ADC. O tempo é reconstruído no receptor e os valores verticais são contagens do ADC simulado. Fonte: captura da execução, 2026.

Ao acionar a chave, o modelo elevou LOP e LON; o firmware registrou desconectados e manteve o LED aceso. A aplicação suspendeu a inclusão dos valores e exibiu a condição de contato interrompido, conforme a Figura 5. O restabelecimento da chave retomou a aquisição aceita pelo receptor e o traçado.

Figura 5. Painel após desconexão simulada: o gráfico está suspenso e a conexão MQTT permanece indicada separadamente. Fonte: captura da execução, 2026.

3.2 Montagem e dados capturados

Figura 6. Execução da montagem no Wokwi com o bloco AD8232 SIMULADO e registro serial da condição de desconexão. O esquema nomeado completo está na Figura 2 e em diagram.json. Fonte: captura da execução, 2026.

Observação | Resultado documentado

Coleta curta | 10 lotes e 250 valores

Comprimento observado | 25 amostras por lote

Intervalo declarado | 4000 microssegundos

Taxa nominal correspondente | 250 Hz

Menor e maior valor no conjunto curto | 1147 e 2166 contagens

Sequência no conjunto curto | Sem lacunas entre os dez lotes capturados

Duração da coleta estendida | 600.346 segundos no receptor

Lotes e valores na coleta estendida | 1682 lotes e 42050 valores

Regressões de índice observadas | 0

Lacunas positivas entre índices | 1143

Os valores mínimo e máximo descrevem somente o conjunto curto capturado. Não constituem faixa garantida, calibração ou amplitude cardíaca. A sequência sem lacunas nesse recorte não permite concluir entrega de 99% para uma sessão mais longa. O contador interno e os índices devem ser interpretados junto aos dados e às condições do simulador.

A coleta estendida registrou a sequência no receptor e as lacunas positivas de índice entre lotes recebidos. Essas lacunas podem resultar de atrasos de aquisição, descarte de lote ou transporte. Sem um registro independente de todos os lotes efetivamente publicados, não representam isoladamente perda de rede. A taxa de entrega não foi inferida.

3.3 Discussão das limitações

O resultado confirma uma cadeia funcional simulada de leitura analógica, transporte e atuação. O modelo não reproduz o condicionamento elétrico do AD8232 nem verifica contato com pessoas. Também não valida o uso em diagnóstico. O ensaio não comprovou as metas anteriores de pelo menos 99% de entrega, atraso inferior a um segundo e recuperação em trinta segundos. Não há relógios sincronizados para inferir latência unidirecional.

A rotina de rede pode bloquear temporariamente a aquisição, e a conexão QoS 0 não confirma entrega. Não há persistência histórica no Dashboard, controle privado dos tópicos ou comandos remotos do LED. A revisão registra essas diferenças em relação à proposta original para não apresentar intenções como resultados. Uma evolução física ou uma exigência de segurança obrigatória demandará implementação e novos ensaios.

3.4 Código dados e vídeo de demonstração

O repositório público reúne firmware, componente personalizado, diagrama, fluxo Node-RED, dados capturados, script de coleta, figuras e instruções de reprodução. O projeto Wokwi é uma demonstração complementar; o repositório contém também os arquivos do receptor e os dados.

Repositório do projeto: https://github.com/LuizFRod/atividade-objetos-inteligentes-ecg

Simulação Wokwi: https://wokwi.com/projects/479896326597353473

Vídeo de demonstração no YouTube não listado: PENDENTE DE INSERÇÃO PELO AUTOR.

Antes da submissão, o autor deve inserir aqui o link real do vídeo, conferir o acesso do avaliador e apresentar funcionamento, hardware simulado, código, MQTT, atuação e resultados. A ausência desse link e do vídeo mantém a entrega incompleta conforme as instruções da disciplina. O roteiro de gravação foi incluído no repositório para facilitar essa etapa.

4 Conclusões

Foi implementado e demonstrado um sistema de visualização de ECG sintético com modelo externo de interface do AD8232, ESP32, publicação MQTT pela Internet e Node-RED. A leitura por ADC, o envio de lotes, a atuação por LED e o tratamento de desconexão simulada foram observados. Os dados e as figuras permitem reproduzir e examinar o comportamento dentro do ambiente de simulação.

A execução não comprova aquisição de ECG físico nem as metas quantitativas da proposta inicial. O transporte seguro permanece pendente de infraestrutura própria, e o vídeo deve ser inserido antes da entrega. A contribuição ao ODS 3 é educacional, ao permitir estudar aquisição, disponibilidade e limites de monitoramento conectado. A transição para hardware físico e a aceitação acadêmica do modelo simulado exigem avaliação específica, sem extrapolar os resultados apresentados.

5 Referências

ANALOG DEVICES. AD8232 Single Lead Heart Rate Monitor Front End. s.d. Disponível em: https://www.analog.com/en/products/ad8232.html. Acesso em: 5 nov. 2026.

ECLIPSE FOUNDATION. mosquitto.conf man page. s.d. Disponível em: https://mosquitto.org/man/mosquitto-conf-5.html. Acesso em: 5 nov. 2026.

ESPRESSIF SYSTEMS. Arduino ESP32 ADC API. s.d. a. Disponível em: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html. Acesso em: 5 nov. 2026.

ESPRESSIF SYSTEMS. ESP32 DevKitC V4 Getting Started Guide. s.d. b. Disponível em: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/hw-reference/esp32/get-started-devkitc.html. Acesso registrado na proposta original: 13 out. 2026.

FLOWFUSE. Chart ui-chart Node-RED Dashboard 2.0. s.d. Disponível em: https://dashboard.flowfuse.com/nodes/widgets/ui-chart.html. Acesso em: 5 nov. 2026.

NAÇÕES UNIDAS BRASIL. Objetivo de Desenvolvimento Sustentável 3 Saúde e Bem Estar. s.d. Disponível em: https://brasil.un.org/pt-br/sdgs/3. Acesso em: 5 nov. 2026.

OASIS. MQTT Version 3.1.1. OASIS Standard, 29 out. 2014. Disponível em: https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html. Acesso em: 5 nov. 2026.

OPENJS FOUNDATION. Importing and Exporting Flows Node-RED. s.d. Disponível em: https://nodered.org/docs/user-guide/editor/workspace/import-export. Acesso em: 5 nov. 2026.

RODRIGUES, Luiz Felipe. Sistema IoT de Monitoramento Cardíaco com ESP32 e Sensor AD8232. Aplicando Conhecimento 3. São Paulo: Universidade Presbiteriana Mackenzie, 2026. Documento base fornecido pelo autor.

SPARKFUN ELECTRONICS. AD8232 Heart Rate Monitor Hookup Guide. s.d. Disponível em: https://learn.sparkfun.com/tutorials/ad8232-heart-rate-monitor-hookup-guide/all. Acesso registrado na proposta original: 13 out. 2026.

WOKWI. Analog API. s.d. b. Disponível em: https://docs.wokwi.com/chips-api/analog. Acesso em: 5 nov. 2026.

WOKWI. Getting Started with the Wokwi Custom Chips C API. s.d. a. Disponível em: https://docs.wokwi.com/chips-api/getting-started. Acesso em: 5 nov. 2026.

Apêndice A Materiais e montagem da proposta física

A proposta anterior especificava ESP32 DevKitC V4, módulo SparkFun AD8232 SEN 12650, três eletrodos, cabo de três vias, LED vermelho, resistor de 330 ohms, protoboard, jumpers e alimentação por power bank de 5 V via USB. Esses itens são mantidos como documentação da concepção física, sem afirmar compra, montagem ou ensaio de bancada. As fotografias e ilustrações deste apêndice foram recuperadas do artigo base fornecido pelo autor, com as atribuições originais.

Figura A1. Referências físicas: ESP32 DevKitC V4 à esquerda e módulo AD8232 à direita. Fontes atribuídas no artigo base: Espressif Systems (s.d. b) e SparkFun Electronics (s.d.). São referências de componentes, não fotografias da execução deste trabalho.

Figura A2. Materiais auxiliares da proposta física original. As ilustrações são de elaboração própria do artigo base, sem escala e sem comprovação de aquisição. O resistor de 330 ohms corresponde à proposta física, distinta do resistor de 220 ohms da simulação.

Na concepção física, OUTPUT seria conectado ao GPIO34, LO+ e LO- aos GPIO32 e GPIO33, SDN a 3,3 V e LED ao GPIO25 com resistor de 330 ohms. O módulo receberia 3,3 V; o ESP32 seria alimentado por uma única entrada USB. As conexões RA, LA e RL seriam conferidas pela identificação de terminais do fabricante. Essa extensão requer revisão elétrica, avaliação de ruído e saturação e procedimentos institucionais antes de qualquer uso com pessoas. Não foi realizada medição de autonomia, alimentação, ruído ou precisão nesta etapa simulada.
