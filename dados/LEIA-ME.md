# Dados de simulação capturados

ensaio-curto.json contém dez mensagens MQTT realmente recebidas; amostras-ensaio-curto.csv apresenta seus 250 valores.

ensaio-600s.json.gz contém o registro completo da coleta estendida, comprimido com gzip. Descompacte com qualquer ferramenta compatível. O arquivo JSON completo também acompanha o pacote local. resumo-ensaio-600s.json contém as estatísticas publicadas no artigo.

A duração de 600,346 s corresponde ao relógio do receptor. Foram recebidos 1682 lotes e 42050 valores, com 1143 lacunas positivas entre índices e nenhuma regressão observada. Essas lacunas incluem os efeitos de aquisição e transporte; não constituem medida isolada de perda de pacotes. A taxa de 250 Hz é nominal e se refere ao tempo simulado.

Os arquivos não contêm ECG de pessoas. O campo eletrodos representa o estado da chave do modelo.
