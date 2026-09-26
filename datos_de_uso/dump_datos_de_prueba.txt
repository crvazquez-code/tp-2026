DUMP DE VERIFICACION — dataset de prueba del TP "Buffet Albert Einstein"
(generado releyendo los .dat binarios; sirve para chequear que la lectura binaria es correcta)

Tamanos de registro (bytes): sizeof(ComandaHistorica)=76  sizeof(Producto)=64
Si tu sizeof no coincide, revisá el orden/padding de los campos.

===== inventario.dat  (ordenado por codigo, CON HUECOS) =====
codigo   descripcion        precio    stock
101      Cafe              1500.00       60
105      Medialuna          800.00       80
110      Tostado           3200.00       30
112      Jugo Naranja      2100.00       40
120      Agua              1200.00       50
125      Alfajor           1800.00       30
130      Sandwich          4500.00       25
140      Gaseosa           1600.00       40

===== comandas_historicas.dat  (30 registros, DESORDENADO) =====
fecha        nombreMozo        codProd   cant   comision
02-06-2025   Sofia Ramirez         101      2     300.00
02-06-2025   Bruno Diaz            105      3     240.00
03-06-2025   Carla Lopez           110      1     320.00
02-06-2025   Diego Sosa            120      2     240.00
04-06-2025   Emma Ruiz             125      1     180.00
03-06-2025   Sofia Ramirez         130      1     450.00
05-06-2025   Bruno Diaz            101      1     150.00
02-06-2025   Carla Lopez           140      2     320.00
06-06-2025   Diego Sosa            112      2     420.00
03-06-2025   Emma Ruiz             105      4     320.00
04-06-2025   Sofia Ramirez         120      1     120.00
07-06-2025   Bruno Diaz            130      1     450.00
05-06-2025   Carla Lopez           101      3     450.00
02-06-2025   Emma Ruiz             110      1     320.00
06-06-2025   Sofia Ramirez         125      2     360.00
04-06-2025   Diego Sosa            140      1     160.00
03-06-2025   Bruno Diaz            112      2     420.00
07-06-2025   Carla Lopez           120      3     360.00
05-06-2025   Emma Ruiz             130      1     450.00
06-06-2025   Bruno Diaz            105      2     160.00
04-06-2025   Carla Lopez           101      1     150.00
02-06-2025   Sofia Ramirez         112      1     210.00
07-06-2025   Diego Sosa            110      2     640.00
05-06-2025   Sofia Ramirez         140      1     160.00
03-06-2025   Diego Sosa            125      1     180.00
06-06-2025   Emma Ruiz             101      2     300.00
07-06-2025   Emma Ruiz             120      1     120.00
04-06-2025   Bruno Diaz            130      1     450.00
05-06-2025   Diego Sosa            105      2     160.00
06-06-2025   Carla Lopez           112      1     210.00
