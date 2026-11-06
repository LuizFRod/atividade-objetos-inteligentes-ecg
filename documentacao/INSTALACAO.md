# ECG com AD8232 SIMULADO

Modelo funcional didático para o Wokwi. Não representa um AD8232 físico, aquisição de biopotenciais de pessoas ou simulação elétrica dos filtros e amplificadores. O sinal continua sintético, mas agora é produzido por um componente externo e lido pelo ADC do ESP32.

## O que foi preparado

- Componente personalizado **AD8232 SIMULADO**, com saída analógica OUTPUT, saídas digitais LOP/LON, habilitação SDN e entrada didática DESC.
- ESP32 lendo OUTPUT no GPIO34 com `analogRead()`. O firmware não gera mais a forma de onda por fórmula.
- Chave para representar contato/desconexão dos eletrodos.
- LED externo no GPIO2 e resistor de 220 ohms: aceso na desconexão; pisca na ausência de MQTT; em conexão normal indica picos por limiar do ADC.
- Publicação de lotes de 25 amostras, intervalo nominal de 4000 microssegundos (250 Hz), no tópico original.
- Novo fluxo Node-RED, em aba própria, que limpa e suspende o gráfico quando recebe `eletrodos: "desconectados"`.

## Teste realizado

O firmware e o componente foram compilados e executados no Wokwi. A publicação MQTT foi observada no monitor serial. A nova aba foi importada e implantada no Node-RED local, mantendo as abas anteriores. O Dashboard exibiu o sinal; ao acionar a chave, exibiu desconexão e suspendeu o traçado; ao restabelecer o contato, a curva retornou. As capturas estão na pasta imagens/.

Esta verificação é funcional e não é ensaio de precisão, perda de pacotes ou validação clínica. Foram observadas lacunas de temporização no firmware, especialmente durante conexão/envio de rede. O contador `perdidas_temporizacao` registra períodos de amostragem perdidos, não todos os dados descartados nem perdas de rede.

## Configuração segura do Node-RED

Após importar, configure o broker ECG seguro • EMQX com usuário painel-ecg e sua senha. Configure ECG • teste publicador TLS com usuário esp32-84729 e sua senha. Nas configurações TLS, carregue seguranca/emqxsl-ca.crt como CA, mantenha a validação e o Server Name igual ao host. A exportação não inclui senhas nem a CA no armazenamento de credenciais.

## Página já adicionada ao Node-RED local

[AD8232 simulado](http://localhost:1880/dashboard-ecg-84729/ad8232)

O Node-RED precisa permanecer rodando e a nova simulação precisa publicar para manter o gráfico atualizado. O Dashboard existente continua em `/dashboard-ecg-84729/ecg`. A nova página usa o mesmo Dashboard com caminho `/ad8232`.

**Não importe novamente se a aba ECG • AD8232 SIMULADO já estiver presente.** O arquivo `node-red/fluxo-ecg.json` é a cópia para restauração ou uso em outra instalação. Em outra instalação com um Dashboard existente em caminho diferente, confira o caminho da página na barra lateral Dashboard 2.0.

## Como abrir os arquivos no Wokwi

A cópia foi salva como projeto público após autorização do autor: https://wokwi.com/projects/479900142449265665. Seu projeto original não foi alterado.

Para restaurar manualmente:

1. Crie um novo projeto ESP32 no Wokwi ou faça uma cópia do seu projeto original.
2. Substitua o conteúdo de `sketch.ino` pelo arquivo desta pasta.
3. Clique em **+ → Custom Chip**. Digite **ad8232**, selecione **C** e clique em **Create Chip**. Esse passo cria as abas `ad8232.chip.c` e `ad8232.chip.json`.
4. Substitua o conteúdo dessas duas abas pelos arquivos de mesmo nome nesta pasta.
5. Substitua `diagram.json` pelo arquivo desta pasta. O circuito inclui ESP32, módulo, chave, LED e resistor.
6. Em **Library Manager**, adicione **PubSubClient**. O arquivo `libraries.txt` desta pasta registra essa dependência.
7. Inicie a simulação e digite a senha de esp32-84729 no campo do monitor serial, pressionando Enter. Aguarde Wi-Fi e NTP. O serial deve registrar `MQTTS CONECTADO` e `MQTT OK`. Não salve a senha no código.
8. No menu do diagrama, use **Fit** para enquadrar todos os componentes.
9. Salve a cópia com nome próprio, confirmando a visibilidade pretendida.

Pare a versão anterior do Wokwi antes de iniciar a nova, pois ambas usam o mesmo tópico/dispositivo. O teste inject manual do Node-RED também usa o mesmo tópico; execute-o separadamente.

## Ligações

| Origem | Destino | Função |
|---|---|---|
| ESP32 3V3 | módulo VCC e SDN | Alimentação representada e habilitação |
| ESP32 GND | módulo GND | Referência representada |
| módulo OUTPUT | ESP32 GPIO34 | Saída analógica sintética → ADC1 |
| módulo LOP | ESP32 GPIO32 | Desconexão simulada, equivalente funcional a LO+ |
| módulo LON | ESP32 GPIO33 | Desconexão simulada, equivalente funcional a LO- |
| ESP32 GPIO2 | resistor 220 Ω → anodo LED | Atuador visual |
| cátodo LED | GND | Retorno |
| chave pino 1 / pino 3 | GND / 3V3 | Seleção do contato simulado |
| chave pino 2 | módulo DESC | Controle didático de desconexão |

LOP/LON são nomes usados pelo componente personalizado para as funções LO+/LO-. **DESC não é um terminal real do AD8232:** é uma entrada de teste exclusiva deste modelo. RA/LA/RL, comportamento de contato humano, impedâncias, circuito de rejeição de modo comum, filtragem e alimentação elétrica não foram modelados. Os fios VCC/GND documentam a montagem pretendida; o modelo não verifica sua alimentação. SDN baixo zera a saída sintética.

## Demonstração

1. Com a chave ligada ao GND, o estado é **conectados**. Abra o novo Dashboard para ver a curva.
2. Clique na chave para ligá-la a 3V3. O módulo coloca LOP/LON em nível alto; o ESP32 acende o LED e publica **desconectados**. A nova página limpa o gráfico e informa **traçado suspenso**.
3. Clique novamente: a indicação retorna a **conectados**, o gráfico volta e o LED passa a indicar picos por limiar.
4. Clique no módulo durante a simulação para ajustar a frequência sintética, entre 30 e 180 BPM. O valor padrão é 75 BPM. Essa frequência é um parâmetro do gerador, não uma frequência cardíaca medida.

## MQTT e formato

Broker `i066e913.ala.us-east-1.emqxsl.com`, porta 8883, MQTT 3.1.1, com TLS, CA/hostname verificados e autenticação, amostras em QoS 0 e sem retain. Tópico: `aula/ecg/esp32-84729/amostras`.

Campos preservados: dispositivo, indice, intervalo_us, eletrodos, amostras. Campos adicionais: `origem: "ad8232-simulado"`, `transporte: "mqtts"`, `sessao` e `perdidas_temporizacao`. O índice corresponde à primeira amostra do lote.

Os lotes marcados como desconectados contêm leituras do ADC, mas essas leituras não entram no gráfico da nova página. Não há fila persistente: lotes podem ser descartados em falhas de rede. Não há comandos MQTT para o LED nesta versão. O gráfico usa horário reconstruído no receptor e não elimina nem quantifica todas as perdas entre lotes.

O Wokwi documenta um ADC analógico simplificado com referência virtual de 5 V. Por isso as contagens da demonstração não equivalem à conversão calibrada de um ESP32 físico. A saída sintética do módulo tem referência de 1,65 V e forma de onda somada a ela; os dados exibidos são contagens do ADC simulado, sem interpretação em mV cardíacos.

## Como descrever no artigo

Use **“modelo funcional de interface do AD8232 com saída sintética”**. Não descreva esta execução como leitura de eletrodos físicos, ECG de paciente ou emulação elétrica validada do chip. A aceitação desse modelo como sensor simulado para a disciplina deve ser confirmada com o professor.

O repositório já está publicado e o artigo contém os links reais. O vídeo YouTube ainda precisa ser gravado e ter seu link inserido na seção 3.4. A conexão TLS autenticada foi implantada e ensaiada; veja seguranca/ATIVACAO-TLS.md.

## Referências técnicas da implementação

- [Custom Chips no Wokwi](https://docs.wokwi.com/chips-api/getting-started)
- [Definição e controles do componente](https://docs.wokwi.com/chips-api/chip-json)
- [Saída analógica do componente e limite do simulador](https://docs.wokwi.com/chips-api/analog)
- [Timers](https://docs.wokwi.com/chips-api/time)
- [Projeto original do autor, preservado](https://wokwi.com/projects/479894329762249729)
