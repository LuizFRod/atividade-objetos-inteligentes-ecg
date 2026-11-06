# Roteiro de vídeo para o YouTube não listado

Duração sugerida de cinco a sete minutos. Gravar a tela e a explicação do autor, sem mostrar senhas ou dados pessoais.

1. Apresentar o objetivo acadêmico, o ODS 3 e o caráter sintético do ECG.
2. Mostrar a montagem: ESP32, bloco AD8232 SIMULADO, saída OUTPUT para GPIO34, LOP/LON para GPIO32/33, chave de contato e LED GPIO2 com resistor 220 ohms.
3. Explicar que o componente externo gera a onda e que o firmware do ESP32 usa analogRead(), reúne 25 amostras e envia o JSON por MQTT. Distinguir o modelo funcional de um sensor físico.
4. Mostrar broker, tópico, mensagens no serial/Debug e gráfico ativo no Node-RED.
5. Acionar a chave: mostrar LED aceso, eletrodos desconectados e suspensão do gráfico. Restaurar o contato e mostrar a retomada.
6. Mostrar as pastas do repositório e os dados capturados. Explicar taxa nominal, lacunas de temporização e ausência de medição clínica.
7. Mostrar a conexão EMQX na porta 8883 com TLS e validação de certificado, usuários separados e o teste manual retornando pelo broker. Explicar que os dados históricos de dez minutos eram do broker público e que a nova coleta TLS possui 372 lotes e 9300 valores. Não mostrar senhas durante a gravação; iniciar a captura depois de informá-las. Explicar a interrupção observada do gateway público do Wokwi e a necessidade de reiniciar a simulação nessa condição.

Após publicar como não listado, testar o vídeo em janela sem login. Inserir o link real na seção 3.4 do artigo e no README do repositório. O artigo sem esse link e sem o vídeo continua incompleto para a avaliação.
