# MQTT com TLS e autenticação

## Implantação utilizada

EMQX Cloud Serverless, host `i066e913.ala.us-east-1.emqxsl.com`, porta 8883, MQTT 3.1.1. O broker utiliza infraestrutura compartilhada do provedor e uma implantação com acesso autenticado do projeto. Não foi instalado um Mosquitto dedicado. O limite de gastos deve permanecer em zero para restringir o uso à franquia gratuita, sujeito às regras do provedor.

## Credenciais

- `esp32-84729`: publicador do ESP32 e do teste manual, com client IDs distintos.
- `painel-ecg`: receptor do Node-RED.
- Senhas exclusivas são fornecidas pelo autor. Nunca são incluídas neste repositório.

O firmware `wokwi/sketch.ino` é a versão TLS ativa; `sketch-tls.ino` é uma cópia de referência idêntica. A CA pública DigiCert Global Root G2 baixada da implantação está incorporada ao firmware e também em `emqxsl-ca.crt`. O ESP32 usa WiFiClientSecure e setCACert, aguarda horário NTP e não usa setInsecure ou porta 1883 como alternativa.

No Wokwi público, digite a senha **no monitor serial em tempo de execução**, nunca na aba do código. O firmware a mantém em RAM, sem eco ou armazenamento em arquivos. Reiniciar apaga essa memória. Um `secrets.h` publicado no Wokwi continuaria público, mesmo que o Git o ignore. O arquivo `secrets.h.example` é somente um exemplo legado para ambiente privado, não é usado pelo firmware atual.

No Node-RED, mantenha as duas configurações MQTT separadas. Carregue a CA pública, habilite a verificação do certificado e informe o hostname acima como Server Name. Informe usuário/senha somente na aba Segurança. Não exporte arquivos de credenciais descriptografados.

## Permissões esperadas no EMQX

Em Access Control → Authorization → Username:

| Username | Topic | Action | Permission |
|---|---|---|---|
| esp32-84729 | aula/ecg/esp32-84729/amostras | Publish | Allow |
| painel-ecg | aula/ecg/esp32-84729/amostras | Subscribe | Allow |

Em All Users, a regra final é Topic `#`, Action `Publish & Subscribe`, Permission `Deny`. A ordem de avaliação do provedor verifica regras específicas e depois a negação geral. Authentication cadastra as senhas; Authorization define as permissões. Não são a mesma etapa.

## Resultados e alcance da verificação

A compilação, o handshake TLS autenticado, a publicação do ESP32, a recepção pelo painel e o teste manual foram executados. A coleta TLS tem 372 lotes e 9300 valores, com janela configurada em 60 s. Um teste independente negociou TLS 1.2 com CA e hostname verificados, recusou MQTT anônimo e senha incorreta (CONNACK 4) e rejeitou hostname divergente. Veja `ensaios/seguranca-tls.json`, `ensaios/serial-tls.txt` e `dados/resumo-ensaio-tls.json`.

Os testes de autenticação não substituem uma auditoria integral de ACL, isolamento entre contas ou segurança da aplicação. As permissões devem ser conferidas no console; testes negativos de publicar/assinar fora da permissão ainda não foram demonstrados neste registro. Não se afirma certificação de segurança ou conformidade clínica.

O gateway público do Wokwi encerrou uma conexão. Reiniciar a simulação e informar a senha novamente restabeleceu o envio. Um teste de disponibilidade contínua e recuperação automática do gateway não foi concluído.

## Reprodução dos testes

Dentro do repositório:

- `python3 ensaios/verificar_seguranca_tls.py`: testa a recusa de acesso anônimo, senha incorreta e hostname divergente; não solicita nem grava senhas reais.
- `python3 ensaios/coletar_mqtt.py --segundos 60 --saida dados/ensaio-tls-novo.json`: solicita a senha do leitor em terminal privado e coleta lotes TLS sem incluí-la nos dados.

A conta do autor não é acesso público ao broker. Para reproduzir independentemente, crie sua implantação e ajuste host, CA e usuários nos arquivos públicos, mantendo as senhas fora deles.

Referências oficiais:
- https://docs.emqx.com/en/cloud/latest/deployments/port_guide_serverless.html
- https://docs.emqx.com/en/cloud/latest/deployments/default_authz.html
- https://docs.emqx.com/en/cloud/latest/price/pricing.html

Os arquivos mosquitto.conf.example e acl.example são alternativa não utilizada neste ensaio.
