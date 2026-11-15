# Ensaio complementar de tempos

Importe fluxo-ecg.json e ensaio-tempos.json no Node-RED. As conexões TLS usam as credenciais locais já configuradas; nenhuma senha acompanha os arquivos. Para outra instalação, configure host, CA e credenciais nos dois nós MQTT. O usuário esp32-84729 deve poder publicar e assinar somente o tópico aula/ecg/esp32-84729/amostras.

Carregue wokwi/sketch.ino e os demais arquivos wokwi no simulador. A URL pública anterior pode conter o firmware anterior: use os arquivos deste pacote para repetir o ensaio. Informe a senha no serial em tempo de execução e mantenha a chave de contato conectada.

A aba de ensaio é entregue desabilitada após a coleta. Habilite-a e aplique Deploy antes de medir. Na raiz do repositório, execute:

```sh
python3 ensaios/medir_tempos.py
```

O programa usa apenas a biblioteca padrão do Python. Envia comandos HTTP ao Node-RED local, que os publica por MQTT TLS. O ESP32 aplica o LED e responde via broker. Ao terminar, AUTO restaura a sinalização normal. Desabilite novamente a aba de ensaio.

Dados reais desta execução: dados/tempos-resposta/eventos.json, medidas.csv e resumo.json. Cada elemento tem quatro medidas. Os tempos unidirecionais são estimativas com intervalos amplos, sob aproximação afim dos relógios; não são medições precisas nem comprovam a meta de latência inferior a um segundo. O RTT é medido diretamente no mesmo relógio. Veja seção 2.7 do artigo.

A revisão está disponível neste repositório. A URL pública Wokwi ainda precisa receber os arquivos revisados antes da entrega; carregue os arquivos da pasta wokwi/ para reproduzir o ensaio atual. O vídeo e a aceitação docente do sensor simulado continuam pendentes.
