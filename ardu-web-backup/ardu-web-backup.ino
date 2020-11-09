#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>

#define RESET 9
#define STOP 8
#define C_GENERADOR 7 //SALIDA AL CONTACTOR DEL GENERADOR
#define C_ANDE 6 //SALIDA AL CONTACTOR DE ANDE
#define START 5

#define S_ANDE 4 //SEÑAL DE ANDE
#define S_GENERADOR 3//SEÑAL DE GENERADOR

// Crear una instancia EnergyMonitor
EnergyMonitor energyMonitor;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Direccion Fisica MAC
IPAddress ip(192, 168, 0, 177);                      // IP Local que usted debe configurar

EthernetServer server(80);                             //Se usa el puerto 80 del servidor

// Voltaje de nuestra red eléctrica
float voltajeRed = 220.0;
int cont_ande = 0;
int cont_generador = 0;
String readString;

void setup() {
  pinMode(RESET, OUTPUT);
  pinMode(STOP, OUTPUT);
  pinMode(C_GENERADOR, OUTPUT);
  pinMode(C_ANDE, OUTPUT);
  pinMode(START, OUTPUT);

  pinMode(S_ANDE, INPUT);
  pinMode(S_GENERADOR, INPUT);


  // Iniciamos la clase indicando
  // Número de pin: donde tenemos conectado el SCT-013
  // Valor de calibración: valor obtenido de la calibración teórica
  energyMonitor.current(0, 2.63);

  Ethernet.begin(mac, ip); // Inicializa la conexion Ethernet y el servidor
  server.begin();

  Serial.begin(9600);

}

void loop()
{

  // Obtenemos el valor de la corriente eficaz
  // Pasamos el número de muestras que queremos tomar
  double Irms = energyMonitor.calcIrms(1500);

  // Calculamos la potencia aparente
  double potencia =  Irms * voltajeRed;

  //Variables donde se almacenan los estados de energia
  //Leemos las señales de entrada de Ande Y Generador
  boolean estado_ande = digitalRead(S_ANDE);
  boolean estado_generador = digitalRead(S_GENERADOR);


  /*--------------------------------- SI TENEMOS ANDE --------------------*/
  //Si tenemos señal de ande
  if (estado_ande) {
    cont_generador = 0;
    digitalWrite(C_GENERADOR, LOW);

    if (cont_ande == 0) {

      //Desactivamos el contactor de generador
      digitalWrite(C_GENERADOR, LOW);
      delay(3000);

      //Activamos el contactor de la Ande
      digitalWrite(C_ANDE, HIGH);

      //Activamos el stop del generador
      digitalWrite(STOP, HIGH);
      cont_ande = cont_ande + 1;
    }
    /*--------------------------------- SI NO TENEMOS ANDE --------------------*/
  } else {
    cont_ande = 0;

    if (cont_generador == 0) {
      //Desactivamos el contactor de la Ande
      digitalWrite(C_ANDE, LOW);

      delay(3000);

      //Activamos el contactor de generador
      digitalWrite(C_GENERADOR, HIGH);

      //Desactivamos el stop del generador
      digitalWrite(STOP, LOW);
      cont_generador = cont_generador + 1;
    }

  }

  // Crea una conexion Cliente
  EthernetClient cliente = server.available();


  if (cliente) {
    //Comprobamos si hay conexion con el cliente
    while (cliente.connected()) {
      //Comprobamos si hay byte para ser leidos
      if (cliente.available()) {
        char c = cliente.read();

        //Lee caracter por caracter HTTP
        if (readString.length() < 100) {
          //Almacena los caracteres a un String
          readString += c;

        }

        // si el requerimiento HTTP fue finalizado
        if (c == '\n') {
          Serial.println(readString); //Imprime en el monitor serial

          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");
          cliente.println("");

          cliente.println("<!DOCTYPE html>");
          cliente.println("<html lang='es'>");
          cliente.println("<head>");
          cliente.println("<meta charset='UTF-8'>");
          cliente.println("<meta name=viewport content=width=device-width, initial-scale=1.0>");
          cliente.println("<title>Web Ardu</title>");
          cliente.println("<style type=\'text/css\'>");

          cliente.println("body{");
          cliente.println("background: gray;");
          cliente.println("}");

          cliente.println(".container{");
          cliente.println("width: 100%;");
          cliente.println("text-align: center;");
          cliente.println("margin: auto;");
          cliente.println("}");

          cliente.println(".container .senhal {");
          cliente.println("width: 100%;");
          cliente.println("height: 25vh;");
          cliente.println("display: inline-flex;");
          cliente.println("}");

          cliente.println("#ande, #generador {");
          cliente.println("width: 80px;");
          cliente.println("height: 80px;");
          cliente.println("border-radius: 50%;");
          cliente.println("background: #5cb85c;");
          cliente.println("}");

          cliente.println("#ande{");
          cliente.println("margin-left: 15%;");
          cliente.println("}");

          cliente.println("#generador{");
          cliente.println("margin: 0 20%;");
          cliente.println("}");

          cliente.println(".display{");
          cliente.println("margin: auto;");
          cliente.println("width: 200px;");
          cliente.println("height: 80px;");
          cliente.println("border: solid 2px #000;");
          cliente.println("text-align: center;");
          cliente.println("}");

          cliente.println(".display b{");
          cliente.println("display: block;");
          cliente.println("margin-top: 12%;");
          cliente.println("font-size: 26px;");
          cliente.println("}");

          cliente.println(".labels{");
          cliente.println("position: absolute;");
          cliente.println("top: 25%;");
          cliente.println("left: 15%;");
          cliente.println("width: 300px;");
          cliente.println("font-weight: bold;");
          cliente.println("}");

          cliente.println("#l-ande{");
          cliente.println("margin-left: -20%;");
          cliente.println("}");

          cliente.println("#l-generador{");
          cliente.println("margin-left: 30%;");
          cliente.println("}");

          cliente.println(".buttons{");
          cliente.println("width: 80%;");
          cliente.println("height: 20vh;");
          cliente.println("margin: 20% auto;");
          cliente.println("}");

          cliente.println(".buttons a{");
          cliente.println("font-size: 22px;");
          cliente.println("width: 300px;");
          cliente.println("height: 50px;");
          cliente.println("background-color: #0066ff;");
          cliente.println("border: solid 2px #000;");
          cliente.println("border-radius: 20px;");
          cliente.println("text-decoration: none;");
          cliente.println("color: #fff;");
          cliente.println("margin: 6%;");
          cliente.println("}");

          cliente.println("a{");
          cliente.println("padding: 20px 25px;");
          cliente.println("}");

          cliente.println("</style>");

          cliente.println("</head>");

          cliente.println("<body>");

          cliente.println("<div class='container'>");
          cliente.println("<h2>TTA REMOTE</h2>");

          cliente.println("<div class='senhal'>");
          cliente.println("<div id='ande'></div>");
          cliente.println("<div id='generador'></div>");
          cliente.println("</div>");

          cliente.println("<div class='labels'>");
          cliente.println("<label id='l-ande'>Ande</label>");
          cliente.println("<label id='l-generador'>Generador</label>");
          cliente.println("</div>");

          cliente.println("<div class='display'>");
          cliente.println("<b>4.5A</b>");
          cliente.println("</div>");

          cliente.println("<div class='buttons'>");
          cliente.println("<a href=\"/?start\"\">Start</a>");
          cliente.println("<a href=\"/?reset\"\">Reset</a>");
          cliente.println("</div>");

          cliente.println("</div>");

          cliente.println("</body>");
          cliente.println("</html>");

          delay(1);
          cliente.stop();

          // Limpia el String(Cadena de Caracteres para una nueva lectura
          readString = "";
          if (readString.indexOf("?reset") > 0) {
            digitalWrite(RESET, HIGH);
          }
        }

        //Reseteo del combustible
        //reset(peticion, cliente);


        //Peticion para poder enviar el valor de la corriente
        //recibirCorriente(peticion, cliente, Irms);

        // Si recibimos la señal de ande, le enviamos un verdadero a la aplicacion
        //statusAnde(peticion, cliente, estado_ande);

        //Enviamos la señal del generador a la aplicacion
        //statusGenerador(peticion, cliente, estado_generador);

        //Si no tenemos ande podemos dar arranque al generador
        //        if (!estado_ande) {
        //          //Si no arranca el generador el va seguir intentado
        //          if (!estado_generador) {
        //            //Peticion para poder encender el generador
        //            start_G(peticion);
        //          }
        //        }


      }
    }
  }
}



void start(int duracion) {
  digitalWrite(START, HIGH);
  delay(duracion);
  digitalWrite(START, LOW);
}

void start_G( String peticion) {
  //Peticion para poder encender el generador
  if (peticion.indexOf("start") >= 0) {
    int duracion = peticion.substring(peticion.indexOf("?") + 6, peticion.indexOf("?") + 10).toInt();
    start(duracion);
  }
}


void statusAnde( String peticion, EthernetClient client,  boolean data) {
  if (peticion.indexOf("estadoande") >= 0) {
    client.print(data);
  }
}

void statusGenerador( String peticion, EthernetClient client,  boolean data ) {
  if (peticion.indexOf("estadogenerador") >= 0) {
    client.print(data);
  }
}

void recibirCorriente(String peticion, EthernetClient client, double corriente) {
  if (peticion.indexOf("enviaDato") >= 0) {
    client.print(corriente);
  }
}

void stopG( String peticion ) {


}

void reset() {
  digitalWrite(RESET, HIGH);
  delay(500);
  digitalWrite(RESET, LOW);
}
