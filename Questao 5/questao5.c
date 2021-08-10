/*
5. Para facilitar e gerenciar os recursos de um sistema computacional com múltiplos processadores (ou núcleos),
você deverá desenvolver uma API para tratar requisições de chamadas de funções em threads diferentes.

API deverá possuir: 


* Uma constante N que representa a quantidade de processadores ou núcleos do sistema computacional.
Consequentemente, N representará a quantidade máximas de threads em execução;


* Um buffer que representará uma fila das execuções pendentes de funções;;


* Função agendarExecucao.
 Terá como parâmetros a função a ser executada e os parâmetros desta função em uma struct.
 Para facilitar a explicação, a função a ser executada será chamada de funexec.
 Assuma que funexec  possui o mesmo formato daquelas para criação de uma thread: um único parâmetro.
 Isso facilitará a implementação, e o struct deverá ser passado como argumento para funexec durante a criação da thread.
 A função agendarExecucao é não bloqueante, no sentido que o usuário ao chamar esta funcionalidade, a requisição será colocada no buffer,
 e um id será passado para o usuário. O id  será utilizado para pegar o resultado após a execução de funexec  e pode ser um número sequencial;


* Thread despachante. 
 Esta deverá pegar as requisições do buffer,
 e gerenciar a execução de N threads responsáveis em executar as funções funexecs. 
 Se não tiver requisição no buffer, a thread despachante dorme. 
 Pelo menos um item no buffer,  faz com que o despachante acorde e coloque a funexec  pra executar. 
 Se por um acaso N threads estejam executando e existem requisições no buffer, 
 somente quando uma thread concluir a execução, uma nova funexec  será executada em uma nova thread. 
 Quando funexec concluir a execução, seu resultado deverá ser salvo em uma área temporária de armazenamento 
 (ex: um buffer de resultados). O resultado de uma funexec deverá estar associada ao id retornado pela função agendarExecucao. 
 Atenção: esta thread é interna da API e escondida do usuário.


* Função pegarResultadoExecucao. 
  Terá como parâmetro o id retornado pela função agendarExecucao.
  Caso a execução de funexec  não tenha sido concluída ou executada,
  o usuário ficará bloqueado até que a execução seja concluída.
  Caso a execução já tenha terminado,  será retornado o resultado da função.
  Dependendo da velocidade da execução, em muitos casos, os resultados já estarão na área temporária.


A implementação não poderá ter espera ocupada, 
e os valores a serem retornados pelas funções funexec podem ser todas do mesmo tipo 
(ex: números inteiros ou algum outro tipo simples ou composto definido pela equipe). 
funexec é um nome utilizado para facilitar a explicação, e diferentes nomes poderão ser utilizados para definir as funções que serão executadas de forma concorrente.
Você deverá utilizar variáveis de condição para evitar a espera ocupada. Lembre-se que essas variáveis precisam ser utilizadas em conjunto com mutexes.
 Mutexes deverão ser utilizados de forma refinada, no sentido que um recurso não deverá travar outro recurso independente.
*/