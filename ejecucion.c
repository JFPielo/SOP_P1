/*-----------------------------------------------------+      
 |     E J E C U C I O N . C                           |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion: En este fichero se encontrará      |
 |     el código que se encargará de ejecutar las      |
 |     línea de comandos en el shell.                  |
 +-----------------------------------------------------*/
#include "defines.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <signal.h>
#include "profe.h"
#include <unistd.h> 
#include <sys/wait.h> 
#include <fcntl.h> 
#include <stdio.h>   
#include <string.h>

//Esto para utilizar la función visualizar que está en ush.c
extern void visualizar(void);




int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{
    //Compruebo si la ejecución es en background
    if(bgnd){
        //Si lo es, creo un proceso auxiliar que me permitirá verificar el estado de los otros procesos en background
        pid_t aux = fork();
        //Comprobación de errores en el fork
        if (aux < 0) {
            return ERROR;
        }
        //Si el proceso shell padre, imprimira el mensaje de ejecución en background y terminará
        if(aux !=0){
            printf("-----Ejecución en segundo plano-----\n");
            fflush(stdout);
            cerrar_fd();
            return OK;
        }
    }
    //Ahora el proceso aux hijo seguira en dado caso se haya ejecutado en background 
    //En foreground no existe ese proceso auxiliar
    //Arrego que almacenara los pids de los procesos
    pid_t pids[PIPELINE];
    //Bucle que recorre las ordenes a ejecutar
    for (int i=0; i<nordenes; i++){
        //Realizo un fork para crear un proceso hijo inicial en foreground y un hijo de auxiliar en background
        pid_t ret = fork();
        //Comprobación de errores en el fork
        if (ret < 0) {
            cerrar_fd();
            if(bgnd){
                _exit(127);
            } else {
                return ERROR;
            }
        }
        //Comprueba que sea el hijo y ejecuta la orden
        if (ret == 0){
            //Redirige entrada a descriptor
            if (redirigir_entrada(i) == ERROR){
                //Utilizo el _exit(126) para indicar error en la redirección del hijo
                //Esto con el fin de no afectar al estado del padre y que termine el proceso actual
                 _exit(126);
            }
            //Redirige salida a descriptor
            if (redirigir_salida(i) == ERROR){
                _exit(127);
            }
            //Cierra tubos y descriptores
            cerrar_fd();
            //Ejecuta los argumentos
            execvp(args[i][0], args[i]);
            //Utilizo el _exit(127) para indicar error en la ejecución del hijo
            //Esto con el fin de no afectar al estado del padre y que termine el proceso actual
            _exit(127);
        } 
        //Si es el padre almacena los pids de los procesos hijos
        else {
            pids[i] = ret;
        }
            
    }
    //Cierra tubos y descriptores del padre
    cerrar_fd();

    //Bucle que espera a que todos los procesos hijos terminen
    for (int i=0; i<nordenes; i++) {
        waitpid(pids[i], NULL, 0);
    }
    //Comprueba si la ejecución es en background e iumprime la tabla de visualización
    if (bgnd) {
        visualizar();
        _exit(0);
    }
    //Si no retorna OK
    else{
        
        return OK;
    }
}       

// Fin de la funcion "ejecutar"
