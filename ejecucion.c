/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion:                                    |
 +-----------------------------------------------------*/
#include "defines.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <signal.h>
#include "profe.h"
#include <unistd.h> 
#include <sys/wait.h> 


int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{
    pid_t ret;
    int status;
    //Bucle que crea 
    for (int i=0; i<nordenes; i++){
        //Crea hijo
        ret = fork();
        //Comprueba si es el hijo o el padre
        if(ret == 0){
            //Redirige entrada a discriptor
            redirigir_entrada(i);
            //Redirige salida a descriptor
            redirigir_salida(i);
            //Cierra tubos y descriptores
            cerrar_fd();
            
            //Ejecuta los argumentos
            execvp(args[i][0],args[i]);  
        }
        };
    cerrar_fd();
    if(bgnd){

        return OK;
    }else{
        if(waitpid(ret,&status,0)==-1){
            return ERROR;
        }
        printf("DONE \n");
        return OK;
    }
    //while(wait(NULL)!= -1);
    //return OK;
    
} // Fin de la funcion "ejecutar"
