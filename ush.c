/*-----------------------------------------------------+
 |     U S H. C                                        
 +-----------------------------------------------------+
 |     Versión :                                       |                      
 |     Autor :   Josue Piedrasanta                     |
 |     Asignatura :  SOP-GIIROB                        |                               
 |     Descripción :  En este fichero se encontrará    |
 |     el código que implementa el shell.              |
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "profe.h"
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>


//
// Declaraciones de funciones locales
//
void visualizar( void );
int leerLinea( char *linea, int maxLinea );

//Recolector de hijos que terminan su proceso
void recolector(void) {
  //Bucle que recolecta a los procesos hijos para que no sean zombies, WNOHANG evita que se bloquee el shell y si
  //no hay hijos el shell sigue respondiendo mientras se sigue ejecutando en segundo plano
  while ((waitpid(-1, NULL, WNOHANG)) > 0) {
    //Mensaje de informa que se acabo el proceso en segundo plano
    printf(" ---DONE BACKGROUND ---\n\n");
    //Vacía el buffer de stdout para poder imprimir mensajes correctamente
    fflush(stdout);
  }
}


//Variable que almacena almacenará el directorio previo, PATH_MAX esta definido en defines.h
static char prev_dir[PATH_MAX]; 


//Función para orden interna cd
static int f_cd(int argc, char **argv) {
    //Variable que almacenará el directorio actual, PATH_MAX esta definido en defines.h
    char dir_act[PATH_MAX];
    //Obtiene el directorio actual y comprueba que no esté vacío
    if(getcwd(dir_act,sizeof(dir_act)) == NULL) {
        perror("cd: getcwd error");
        //Establece el directorio actual como vacío
        dir_act[0] = '\0';
    }

    //Variable que almacenará el directorio al que se quiera cambiar
    char *dir_next = NULL;
    //Comprueba que el cd está vacío o no tiene argumentos  
    if (argc < 2) {
        //Asigno a dir_next, que es un puntero, el valor del entorno HOME
        dir_next = getenv("HOME");
        //Comprueba que el entorno HOME no esté definido
        if (dir_next == NULL) {
            fprintf(stderr, "cd: HOME no definido\n");
            return -1;
        }
    }
    //Comprueba si el argumento del cd es "-"
    else if (strcmp(argv[1], "-") == 0) {
        
        //Comprueba si no hay directorio previo almacenado
        if (prev_dir[0] == '\0') {
            fprintf(stderr, "cd: NO SE ESTABLECIO DIRECTORIO PREVIO\n");
            return -1;
        }
        //Si lo es imprime el directorio antiguo
        printf("%s\n", prev_dir);

        //Establece el directorio siguiente como el previo
        dir_next = prev_dir; 
    } 
    //Si no es "-" ni vacío entonces es un directorio normal
    else {
        //Asigno el argumento del cd 
        dir_next = argv[1];
        //Comprueba si el directorio siguiente está vacío
        if (dir_next == NULL || dir_next[0] == '\0') {
            fprintf(stderr, "cd: ruta vacía\n");
            return -1;
        }
    }
    //Donde sucede la magia tras comprobar que es lo quiere hacer con el cd
    //Por lo cual, decido comprobar que el chdir se ejecute correctamente sino imprimo un error
    if (chdir(dir_next) == -1) {
        perror("cd");
        return -1;
    }
    //Se actualiza el directorio previo por el actual y colocó un null al final por seguridad
    if(dir_act[0] != '\0') {
      //Como no se puede asignar un arreglo luego de declararlos, utilizo strncpy para copiar los directorios
        strncpy(prev_dir, dir_act, PATH_MAX);
        prev_dir[sizeof(prev_dir)-1] = '\0';
    }
    return OK;
}




//
// Prog. ppal.
// 
int main(int argc, char * argv[])
{
  char line[255];
  int res;
  char **m_ordenes;
  char ***m_argumentos;
  int *m_num_arg;
  int m_n;

  while(1)
  {
    //Recolector de procesos hijos
    recolector();

    do {
      res = leerLinea(line, MAXLINE);
      if (res == -2) {
        fprintf(stdout,"logout\n");
        exit(0);
      }
      if (res == -1) {
        fprintf(stdout,"linea muy larga\n");
      }
    } while(res <= 0);

    if (analizar(line) == OK) {
      m_n = num_ordenes();
      m_num_arg = num_argumentos();
      m_ordenes = get_ordenes();
      m_argumentos = get_argumentos();

        if (m_n > 0) {
          //Compruebo que exista una sola orden y no sea en background
          if(m_n == 1 && !es_background()){
            //Compruebo que la orden sea cd
            if(strcmp(m_argumentos[0][0],"cd")==0) {
              //Compuebo que la función f_cd se ejecute correctamente
              if(f_cd(m_num_arg[0], m_argumentos[0])!=OK){
                continue;
              }
            }
            //Compruebo que la orden sea exit
            else if(strcmp(m_argumentos[0][0],"exit")==0){
              //Imprime un mensaje para distinguir la salida exit
              printf("logout\n");
              exit(0);
            }
            visualizar();
            recolector();
            
          }
          
          else{
          
            if (pipeline(m_n, fich_entrada(), fich_salida(), es_append(), es_background()) == OK) {
              ejecutar(m_n, m_num_arg, m_ordenes, m_argumentos, es_background());
            }

            //Comprueba que la orden actual no sea background para visualizar
            if (es_background()==0) {
              visualizar();
            }
          }
      }
    }
  }
  return 0;
}





/****************************************************************/
/*                       leerLinea                             
  --------------------------------------------------------------
                                                               
   DESCRIPCIÓN:                                                 
   Obtiene la línea de órdenes para el mShell.    
   Util para depuracion.                                        
                                                                
   ENTRADA:                                                 
    linea - puntero a un vector de carácteres donde se almancenan los caracteres 
   leídos del teclado
    tamanyLinea - tamaño máximo del vector anterior

   SALIDA:
    -- linea - si termina bien, contiene como último carácter el retorno de carro.
    -- leerLinea -  Entero que representa el motivo de finalización de la función:
     > 0 - terminación correcta, número de caracteres leídos, incluído '\n'
     -1 - termina por haber alcanzado el número máximo de caracteres que se 
    espera leer de teclado, sin encontrar '\n'.
     -2 - termina por haber leído fin de fichero (EOF).
*/
/****************************************************************/
//char * getline(void)
int leerLinea( char *linea, int maxLinea )
{
  char* directorio;
  int caracter;
  int i = 0;

  //Obtiene el directorio actual
  directorio = getcwd(linea,maxLinea);
  //Verifica que el directorio se obtuvo de forma correcta
  if(directorio){
    //Muestra el prompt
    printf("%s%s",directorio,PROMPT);
    //Bucle que permite recorrer cada carácter de la cadena
    while(1){
      //Toma el primer carácter del buffer
      caracter = getchar();
    
        //Comprueba si el usuario ha ingresado: ctrl + D
        if(caracter==EOF){
          return -2;
        }

        //Comprueba que la cantidad de carácteres analizados no sea mayor a límite dado
        if ( i >= (maxLinea -1)){
          linea[i] = '\0';
          return -1;
        }
        
        //Almacena en linea el carácter tras convertirlo en char
        linea[i] = (char)caracter;

        //Suma al contador
        i++;

        //Comprueba que el carácter sea un '\n', si lo es procede a ejecutar el break
        if (caracter == '\n'){
          break;
        }
        
    }
    //Coloca de último '\0' para poder utilizar de manera correcta la linea posteriormente
    linea[i] = '\0';
  }
  else{
    printf("%s","No se ha podido Obtener el directorio actual");
    return -1;
  }

  //Retorna la cantidad de carácteres leídos
  return i;
}


/****************************************************************/
/*                       visualizar                             */
/*--------------------------------------------------------------*/
/*                                                              */
/* DESCRIPCIÓN:                                                 */
/* Visualiza los distintos argumentos de la orden analizada.    */
/* Util para depuracion.                                        */
/*                                                              */
/* ENTRADA: void                                                */
/*                                                              */
/* SALIDA: void                                                 */
/*                                                              */
/****************************************************************/
void visualizar( void )
{  
  
  printf("*************************************\n");
  printf("*         👀Visualizador👀          *\n");
  printf("*************************************\n");

  //Imprime el número de ordenes
  printf("➡️  Numero de ordenes: %d \n",num_ordenes());

  int *num_arg = num_argumentos();
  char ***args=get_argumentos();

  //Imprime las ordenes a tráves de dos bucles for que recorren los argumentos de cada orden
  for (int i = 0; i < num_ordenes(); i++)
  {
    //Imprime el número de orden
   printf("\n➡️  Orden: %d \n",i);
   
   //Este bucle recorre la matriz e imprime los argumentos de cada orden
   for (int j = 0; j < num_arg[i]; j++)
   {
    printf("   Argumento: %d ➡️  ",j);
     printf(" %s\n",args[i][j]);
   }
  }
  
  //Verifica que exista redirección de entrada a través de fich_entrada()
  if (*fich_entrada() != '\0'){
    printf("\n➡️  ✅Redireccion de entrada: %s\n",fich_entrada());
  } else {
    printf("\n➡️  ❌No hay redireccion de entrada❌ \n");
  }

  //Verifica que exista redirección de salida a través de fich_salida() y el tipo del mismo con es_append()
  if (*fich_salida() != '\0'){
    if(es_append()){
      printf("\n➡️  ✅Redireccion de Salida modo APPEND: %s\n",fich_salida());
    }else{
    printf("\n➡️  ✅Redireccion de Salida modo TRUNC: %s\n",fich_salida());}
  } else {
    printf("\n➡️  ❌No hay redireccion de salida❌ \n");
  }

  //Verifica si la orden está en Background o Foreground a través de es_background()
  if (es_background()){
    printf("\n➡️  Orden en Background\n");
  } else {
    printf("\n➡️  Orden en Foreground\n\n");
  }
  
} // Fin de "visualizar"



