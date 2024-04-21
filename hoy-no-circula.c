#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

char* contingenciaAnterior="0";    // Variable global saber si cambio el valor de la contingencia

/*OBTIENE EL ULTIMO DIGITO DE UNA PLACA SEGUN LA REGIO (LOCAL O FORANEO ) */
int obtenerUltimoDigito(char* placa, char* region) {  //char* representan un arreglo
    char caracter []="L";
    int numero;
    if(((strcmp(caracter,region)) == 0)) { // Nota strcmp funciona con arreglos
        numero =placa[6]-'0';         // para convertir en int
        return numero;
    } else {
        numero= placa[2]-'0';   // si es foraneo toma el lugar[2]
        return numero;
    }
}

/*FUNCION QUE ES LLLAMADA POR UNA SIGNAL CAMBIA EL VALOR DE LA CONTINGENCIA*/
void signal_callback_handler(int signum)
{
    FILE *archivo;
    char buffer[1024];
    printf("\n\nAtrapo la señal %d\n\n",signum);
    char *env= getenv("ESTADO");  //lle la variable de entorno "ESTADO"
    archivo = fopen(env, "w+"); //se pone el w+  para borrar su contenido antes de escribir , si no existe el archivo lo crea
    fprintf(archivo,"1");   // guarda en el archivo abierto
    fclose(archivo);//Cerrar el archivo
}

/*FUNCION QUE ES LLLAMADA POR UNA SENIAL CAMBIA EL VALOR DE LA CONTINGENCIA*/
void signal_callback_handler2(int signum)
{
    printf("\n\nAtrapo la señal %d\n\n",signum);
    FILE *archivo;
    char buffer[1024];
    char *env= getenv("ESTADO");
    archivo = fopen(env, "w+");
    fprintf(archivo,"0");
    fclose(archivo);
}

/*ABRE EL ARCHIVO QUE DICE LA VARIABLE DE ENTORNO Y LEE SI HAY CONTINGENCIA  */
char *hayContingencia() {
    FILE *archivo;
    char buffer [1024];
    char *env= getenv("ESTADO");
    archivo = fopen(env, "r");

    fgets(buffer, sizeof(buffer), archivo);
    fclose(archivo);
    return strdup(buffer);   //strdup crea una copia de la cadena pasada y la guarda en otro lugar de
    //memoria dinamicamente , lo ocupe porque no me dejaba regresar el buffer

}

/*FUNCION AUXILIAR */
//Regresa 1 si es par
int esPar(char* algo) {
    int espar=0;
    if(atoi(algo)==0) {
        return 1;
    }
    if( (atoi(algo))%2==0) {
        return 1;
    }
    return 0;
}

char* obtenerColor(char* placa,char* region) {
    int ultimo_digito=obtenerUltimoDigito(placa,region);
    if(ultimo_digito==5 || ultimo_digito==6) {
        return "Amarillo";
    }
    if(ultimo_digito==7 || ultimo_digito==8) {
        return "Rosa";
    }
    if(ultimo_digito==3 || ultimo_digito==4) {
        return "Rojo";
    }
    if(ultimo_digito==1 || ultimo_digito==2) {
        return "Verde";
    }
    if(ultimo_digito==9 || ultimo_digito==0) {
        return "Azul";
    }

}

/*RETORNO 0 o 1 SEGUN Y ULTIMO DIGITO DE LA PLACA Y EL DIA CORRESPONDIENTE*/
int verificarEntreSemana(int ultimoDigito,char* dia) {
    int diaa =atoi (dia);  // porque switch no permite comparar cadenade de caracteres

    switch (diaa) {
    case 1: // si es lunes
        if(ultimoDigito==5 ||ultimoDigito==6) {
            return 0;
        }
        return 1 ;
    case 2: // si es martes
        if(ultimoDigito==7 ||ultimoDigito==8) {
            return 0;
        }
        return 1 ;
    case 3:
        if(ultimoDigito==3 ||ultimoDigito==4) {
            return 0;
        }
        return 1 ;
    case 4:
        if(ultimoDigito==1 ||ultimoDigito==2) {
            return 0;
        }
        return 1 ;
    case 5:   //si es viernes
        if(ultimoDigito==9 ||ultimoDigito==0) {
            return 0;
        }
        return 1 ;
    default:
        return 0 ;
    }

}

/*IMPRIME EL REGLON YA CON TODOS LOS ELEMENTOS QUE DEBEN DE ESTAR Y ADEMAS
CREA EL TSV salida.tsv*/
void imprimirRenglon(char* fecha,char* dia, char* placa,char*region,char* holograma,int circula) {

    char circulastr[]="N";
    if(circula==0) { // para traducir el 1 a S , 0=N
        strcpy(circulastr,"N");
    } else {
        strcpy(circulastr,"S");
    }

    printf("%s  %s  %s  %s  %s  %s  %s \n",fecha,dia,placa,region,holograma,obtenerColor(placa,region),circulastr );
    FILE *archivo;
    archivo = fopen("salida.tsv", "a"); //se pone el w+  para borrar su contenido antes de escribir , si no existe el archivo lo crea
    fprintf(archivo,"%s\t%s\t%s\t%s\t%s\t%s\t%s\n",fecha,dia,placa,region,holograma,obtenerColor(placa,region),circulastr );  // el \t es para dejar
    //espacio de un tabular
    //asi como el tsv lo pide

    fclose(archivo);//Cerrar el archivo
}

/*SE ENCARGA DE REALIZAR TODO EL PROCESO LOGICO DEL PROGRAMA **/
void realizarProceso(char* semana, char* dia,char* fecha) {

    int contador=1;
    size_t tamanio=64;
    char region_anterior[tamanio];
    bzero(region_anterior,tamanio);
    char region[tamanio];
    bzero(region,tamanio);
    char placa_anterior[tamanio];
    bzero(placa_anterior,tamanio);
    char placa[tamanio];
    bzero(placa,tamanio);
    char holograma[tamanio];
    bzero(holograma,tamanio);
    int circula=0;  // representa el booleano de si circula o no ,1= circula
    int ultimoDigito=0;

    size_t pos = strcspn(dia, "\n");
    dia[pos] = '\0';// Reemplaza el salto de línea por el carácter nulo terminador

    //Se ocupa contador==1 para que lo haga almenos una vez , la logica es leer ,hasta que se repita el renglon
    while(  !((strcmp(region_anterior,region) == 0)&& (strcmp(placa_anterior,placa) == 0))      || contador==1)
    {
        strcpy(placa_anterior, placa);
        strcpy(region_anterior, region);
        scanf("%s", (char *) &placa);   // es para leer de la entrada estandar , (el tipo de datos, donde se guardara la informacion)
        scanf("%s",(char *) &region);
        scanf("%s",(char *) &holograma);
        if (  !((strcmp(region_anterior,region) == 0)&& (strcmp(placa_anterior,placa) == 0))) // Pongo este
        {   // if porque lee 2 veces la linea final del documento
            ultimoDigito=obtenerUltimoDigito(placa,region);

            if(strcmp(hayContingencia(),"0")==0)// si no hay contingencia
            {
                if(strcmp(holograma,"E")==0 ||(strcmp(holograma,"0")==0) || (strcmp(dia,"7")==0) ) {
                    circula=1;
                    imprimirRenglon(fecha,dia,placa,region,holograma,circula);

                } else
                {
                    if(strcmp(holograma,"1")==0)
                    {
                        if(strcmp(dia,"6")==0) // si es sabado y holograma 1
                        {
                            if(!(esPar(semana)==1)) { //si semana es impar y ultimo digito es impar ,no circula
                                if(!(ultimoDigito==0 || ultimoDigito%2==0)) {
                                    circula=0;
                                    imprimirRenglon(fecha,dia,placa,region,holograma,circula);

                                } else {
                                    circula=1;
                                    imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                                }
                            } else if(ultimoDigito==0 || ultimoDigito%2==0) {
                                circula=0;
                                imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                            } else {
                                circula=1;
                                imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                            }

                        }
                        else { //Si es holograma 1 y entresemana
                            circula=verificarEntreSemana(ultimoDigito,dia); //verifica respecto al dia de la semana
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        }
                    } else {
                        //Para holograma 2 y foraneo
                        if(strcmp(dia,"6")==0) {
                            circula=0;
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        } else {
                            circula=verificarEntreSemana(ultimoDigito,dia); //verifica respecto al dia de la semana
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        }

                    }

                }

            }

            else  //Si hay contingencia
            {

                if(strcmp(holograma,"E")==0 ) {
                    circula=1;
                    imprimirRenglon(fecha,dia,placa,region,holograma,circula);

                } else
                {
                    if(strcmp(holograma,"0")==0)
                    {
                        if( (strcmp(dia,"6")==0)||(strcmp(dia,"7")==0)) // si es sabado y o domingo en holograma 0
                        {
                            circula=1;
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        } else {
                            circula=verificarEntreSemana(ultimoDigito,dia); //verifica respecto al dia de la semana
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);

                        }
                    }
                    else if(strcmp(holograma,"1")==0 ) {

                        if(!(esPar(semana)==1)) { //si semana es impar y ultimo digito es impar ,no circula
                            if(!(ultimoDigito==0 || ultimoDigito%2==0)) {
                                circula=0;
                                imprimirRenglon(fecha,dia,placa,region,holograma,circula);

                            } else {
                                circula=1;
                                imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                            }
                        } else if(ultimoDigito==0 || ultimoDigito%2==0) {
                            circula=0;
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        } else {
                            circula=1;
                            imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                        }

                    } else { //Si es holograma 2 o foraneos no circula ningun dia
                        circula=0;
                        imprimirRenglon(fecha,dia,placa,region,holograma,circula);
                    }
                }

            }

            contador=0;

            contingenciaAnterior= hayContingencia();  //establecemos en la variable global el valor de la contingencia
            //con que hizo el proceso
        }
    }
}

int main(int argc, char* argv[], char* envp[])
{

    printf("My pid is: %d\n", getpid());
    signal(SIGUSR1, signal_callback_handler); // int= 10   // Nota:Alparecer no importa donde ponga este codigo , si atrapa la señal
    // 10 = hay contingencia
    signal(SIGUSR2, signal_callback_handler2);  //int =12                           //aunque el codigo este dentro de un bucle.

    //Configuro la variable de entorno y crea el archivo al que hace referencia la env
    if (setenv("ESTADO","contingencia.txt", 1) != 0) { // El 1 es para establecer el overwrite si es que ya existe la variable
        // En caso de error
        perror("Error al modificar variable de entorno  ");
        return 1;
    }

    if (access("contingencia.txt", F_OK) == -1) {//se verifica si ya existe el archivo para no borrar su contenido cada ejecucion
        FILE *archivo2;
        archivo2 = fopen("contingencia.txt", "w");
        fprintf(archivo2,"0"); //.
        fclose(archivo2);

    }

    FILE *archivo;
    char dia[1024];
    bzero(dia, 1024);
    char semana[1024];

    /* Leyendo los argumentos de entrada
    Leer fecha que es pasado por argumentos */

    //Para sacar el dia de la semana
    char buffer[1024];
    char inicioFecha[]= "date '+%u' --date=";  // Nos dice el dia de la semana partiendo con lunes 1 - domingo 7
    strcat(inicioFecha,argv[1]);   //strcat para concatenar
    strcat(inicioFecha," > resultado.txt");
    system(inicioFecha);  //ejecuto el comando de linux ya con la estructura

    archivo = fopen("resultado.txt", "r"); //Para leer un archivo que se creo al ejecutar el comando anterior
    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {  // lee hasta encontrar un salto de linea
        strcpy(dia,buffer);
    }
    fclose(archivo);

    //Para sacar el numero de semana que es
    bzero(buffer, 1024);
    strcpy (inicioFecha, "date '+%V' --date=");
    strcat(inicioFecha,argv[1]);
    strcat(inicioFecha," > resultado.txt");
    system(inicioFecha);
    archivo = fopen("resultado.txt", "r");

    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        strcpy(semana,buffer);
    }
    fclose(archivo);

    realizarProceso(semana,dia,argv[1]);

    printf("\n\n\n\n");

    //Lo siguiente se realiza para esperar hasta que llegue una signal
    while(1)
    {
        if(strcmp(contingenciaAnterior,hayContingencia())!=0) {
            printf("Se registro un cambio en la contingencia  , vuelve a ejecutar el archivo ");
            return EXIT_SUCCESS;

        } else {
            printf("Esperando una señal .\n");
        }
        sleep(1);
    }

}
