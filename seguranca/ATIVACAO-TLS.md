# Conexão segura preparada para implantação

Estes arquivos são modelos, não uma conexão já implantada. O autor ainda não possui broker próprio com conta configurada. O ensaio e o artigo registram HiveMQ público na porta 1883, sem TLS/autenticação.

Para ativar a especificação original:

1. Disponibilize um broker sob seu controle com DNS, certificado válido e acesso pela Internet. Não exponha o editor Node-RED como forma de publicar o broker.
2. Em Mosquitto, adapte `mosquitto.conf.example` e `acl.example`. Crie senhas individuais com `mosquitto_passwd`, sem gravá-las no histórico de comandos. Autorize somente os tópicos usados.
3. Use `sketch-tls.ino` como variante de firmware, ao lado dos mesmos arquivos do componente e diagrama. Copie `secrets.h.example` para `secrets.h` e configure no ambiente privado o DNS, usuário, senha e CA. O arquivo real secrets.h é ignorado pelo Git. A variante foi compilada no Wokwi com as definições vazias incorporadas ao sketch para o teste e iniciou indicando MQTT bloqueado. O handshake TLS e a autenticação ainda não foram ensaiados com um provedor, por faltar a configuração real; devem passar por esse teste antes de serem apresentados como funcionando.
4. O firmware seguro utiliza WiFiClientSecure, valida a CA e aguarda horário NTP para a verificação do certificado. Não usa setInsecure() nem conexão em texto claro como alternativa.
5. No MQTT broker do Node-RED, informe host/porta correspondentes, habilite TLS e validação do certificado de servidor e configure um usuário exclusivo do painel. Guarde as credenciais no ambiente local; não exporte o arquivo de credenciais descriptografado.
6. Confirme publicação e recepção; teste recusa de senha incorreta e certificado não confiável. Registre resultados, versões e tempos. Atualize o artigo somente depois desse ensaio.

A porta 8883 isoladamente não habilita criptografia. Usuário/senha em broker de teste compartilhado também não demonstram controle privado dos tópicos.

Referência de configuração: https://mosquitto.org/man/mosquitto-conf-5.html
