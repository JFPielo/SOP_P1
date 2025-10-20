/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Version    :                                    |
 |     Autor :   Josue Piedrasanta                     |
 |     Asignatura :  SOP-GIIROB                        |                               
 |     Descripcion: En este fichero se encontrará el   |
 |     código que se encargará de implementar las      |
 |     redirecciones de entrada y salida como la       |
 |     creación de tuberías.                           |
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

REDIRECCION_ORDENES red_ordenes;

//Inicializa los valores de la estructura dejando un 0 en la entrada y un 1 en la salida
void redireccion_ini(void)
{
    for (int i = 0; i<PIPELINE-1;i++){
        red_ordenes[i].entrada = 0;
        red_ordenes[i].salida = 1;
    }
    
}//Inicializar los valores de la estructura cmdfd



int pipeline(int nordenes, char * infile, char * outfile, int append, int bgnd)
{

    //Comprobación de errorers
    if(nordenes > PIPELINE -1){
        return ERROR;
    }

    if(nordenes < 1){
        return ERROR;
    }


    //Inicializa estructura
    redireccion_ini();
    //Inicializa arreglo
    int fd[2];
    //Crea la conexión de las tuberias, por lo cual conecta la orden actual con la siguiente, de la siguente manera en la salida deja la "escritura" de la fila actual y luego
    //pasa a la siguiente orden y deja la "lectura" en la entrada
    for (int i = 0; i < nordenes - 1; i++){
        if(pipe(fd)== -1){
            return ERROR;
        }
        red_ordenes[i].salida = fd[1];
        red_ordenes[i+1].entrada = fd[0];
    }

     //Comprueba que infile no este vacio, por lo cual si no lo esta abre el archivo y lo asigna en la posicion 0 de la entrada
    if(strcmp(infile,"")!= 0){
        int ent = open(infile,O_RDONLY);
        if (ent == -1){
            return ERROR;
        }
        red_ordenes[0].entrada = ent;
    }  

    //Comprueba si la ejecución es en background y no hay redirección de entrada
    else if (bgnd) {
        //Si es así, comprueba si la entrada estándar del shell es un terminal 
        if (isatty(STDIN_FILENO)) {
            //Si lo es, usa /dev/null como entrada para la primera orden y evitar que lea del teclado en background
            int bg = open("/dev/null", O_RDONLY);
            if (bg != -1) {
                red_ordenes[0].entrada = bg;
            }else{
                return ERROR;
            }
        }
    }

    //Verifico el tipo de salida
    int flags;
    if (append != 0){
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }else{
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    }
    
     //Comprueba que outfile no este vacio, por lo cual si no lo esta abre el archivo y lo asigna en la posicion final de la salida
    if(strcmp(outfile,"")!= 0){
        int sal = open(outfile,flags,0666);
        if (sal == -1){
            return ERROR;
        }
        red_ordenes[nordenes-1].salida = sal;
    }

    return OK;
} // Fin de la función "pipeline"


//Redirige la entrada estandar al descriptor
int redirigir_entrada(int i)
{
    int new = red_ordenes[i].entrada;
    if(dup2(new, STDIN_FILENO) == -1){
        return ERROR;
    }
    return OK;
    
} // Fin de la funci�n "redirigir_entrada"


//Redirige la salida estandar al descriptor
int redirigir_salida(int i)
{
    int new = red_ordenes[i].salida;
    if(dup2(new, STDOUT_FILENO) == -1){
        return ERROR;
    }
    return OK;
    
} // Fin de la funci�n "redirigir_salida"

//Cierra todos los tubos y descriptores ya utilizados
int cerrar_fd()
{
    for(int i =0 ; i < PIPELINE-1; i++){
        if (red_ordenes[i].entrada > STDERR_FILENO){
            close(red_ordenes[i].entrada);
            red_ordenes[i].entrada = 0;
        }
        if (red_ordenes[i].salida > STDERR_FILENO){
            close(red_ordenes[i].salida);
            red_ordenes[i].salida = 0;
        }
    }
    return OK;

} // Fin de la funci�n "cerrar_fd"


