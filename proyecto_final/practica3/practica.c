/************************************************** 
*  PRACTICA 3: Realidad Virtual y Aumentada       *
*  Autor: Miguel de la Cal Bravo                  *
*  Máster Universitario en Ingeniería Informática *
***************************************************/

/* LIBRERIAS AR */
#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>
#include <AR/arMulti.h> // Multipatrón
#include <math.h>       // Calcular rotaciones
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
//#include <AR/matrix.h>

// ==== Definicion de constantes y variables globales ===============
int    pattid;                    // Identificador unico de la marca
double patt_trans[3][4];          // Matriz de transformacion de la marca
ARMultiMarkerInfoT *mMarker;      // Estructura global Multimarca

int velocidad = 0;                // Velocidad del juego
double distancia;                 // Distancia entre el objeto 0 y 1
int puntuacion = 0;               // Puntuación obtenida en el juego
int time_to_respawn = 4;          // Tiempo en el que vuelven a aparecer más enemigos
float game_time = 0.0;
float last_respawn_time = 0.0;
int state_enemies[sizeof(mMarker->marker_num)]; // Estado de los enemigos
int current_enemies = 0;          // Número de enemigos actuales en el tablero
int max_enemies = 0;              // Número de enemigos máximo en el nivel de velocidad actual


void print_error (char *error) {  
  printf("%s\n",error);
  exit(0); 
}

// ======== cleanup =================================================
static void cleanup(void) {   // Libera recursos al salir ...
  arVideoCapStop();   arVideoClose();   argCleanup();   exit(0);
}

// ==== Definicion de estructuras ===================================
struct TObject{
  int id;                               // Identificador unico de la marca
  int visible;                          // Es visible el objeto
  double width;                         // Ancho del patron
  double center[2];                     // Centro del patron
  double patt_trans[3][4];              // Matriz de transformacion de la marca
  void (* drawme)(float,float,float);   // Puntero a funcion drawme
  char color;
  float colour[3];
};

struct TObject *objects = NULL;
int nobjects = 0;

// ==== addObject (Agrega un objeto a la lista de objetos) ==============
void addObject(char *p, double w, double c[2], void (*drawme)(float,float,float)){
  int patt_id;

  if((patt_id = arLoadPatt(p)) < 0)
    print_error ("Error en carga de patron\n");

  nobjects++;
  objects = (struct TObject *)
    realloc(objects, sizeof(struct TObject)*nobjects);

  objects[nobjects-1].id = patt_id;
  objects[nobjects-1].width = w;
  objects[nobjects-1].center[0] = c[0];
  objects[nobjects-1].center[1] = c[1];
  objects[nobjects-1].drawme = drawme;

}

// ======== keyboard ================================================
static void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  
  // Recogemos los eventos de pulsaciones de teclas del teclado
  case 0x1B: case 'Q': case 'q':
    cleanup(); break;
  }
}


// Generación aleatoria justa entre todo el tablero de 12 posiciones
static void spawn_enemies ( int id ) {
  if((rand() % (12/max_enemies)) == 0) {
    state_enemies[id] = 1; // Se añade el enemigo al tablero
    current_enemies++;     // Se incrementa el número de enemigos actuales

    // Guardamos el instante de tiempo en el que llegamos al máximo de enemigos
    if (current_enemies == max_enemies) {
      last_respawn_time = game_time;
      printf("LAST: %f TIME: %f", last_respawn_time, game_time);  
    }
  }
}

// ======== draw ====================================================
static void draw( void ) {
  double  gl_para[16];   // Esta matriz 4x4 es la usada por OpenGL
  // La bomba va cambiando de color
  GLfloat mat_ambient[]     = {(fmod(arUtilTimer(), (double) 6)/6), 0.0, 0.0, 1.0};
  GLfloat light_position[]  = {100.0, -200.0, 200.0, 0.0};
  
  /* Pintamos todos los objetos visibles */
  if (objects[0].visible == 1) {
    argDrawMode3D();              // Cambiamos el contexto a 3D
    argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
    glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
      
    argConvGlpara(objects[0].patt_trans, gl_para);   // Convertimos la matriz de
    glMatrixMode(GL_MODELVIEW);                      // la marca para ser usada
    glLoadMatrixd(gl_para);                          // por OpenGL
      
    // Esta ultima parte del codigo es para dibujar el objeto 3D
    glEnable(GL_LIGHTING);  glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
      glTranslatef(0.0, 0.0, 10.0);
      glRotatef(0.0, 1.0, 0.0, 0.0);
      glutSolidCone(40, 60, 50, 50);
    glDisable(GL_DEPTH_TEST);
  }
}

static void draw_sphere(float angle) {
  double  gl_para[16];   // Esta matriz 4x4 es la usada por OpenGL
  GLfloat material[]        = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_position[]  = {100.0, -200.0, 200.0, 0.0};
  float radius;
  float color;

  // Calculamos el radio y color de la esfera, en función del ángulo y la velocidad de juego
  radius = ((angle / velocidad) / 90) * (velocidad * 10) + 10;
  color = angle / 90;

  if (objects[1].visible == 1) {
    argDrawMode3D();              // Cambiamos el contexto a 3D
    argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
    glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
      
    argConvGlpara(objects[1].patt_trans, gl_para);   // Convertimos la matriz de
    glMatrixMode(GL_MODELVIEW);                      // la marca para ser usada
    glLoadMatrixd(gl_para);                          // por OpenGL
      
    // Dibujamos el objeto con colores entre rojo y verde, según la velocidad
    if (velocidad == 1) {
      material[0] = 0.0; material[1] = color; material[2] = 0.0;
    }else if (velocidad == 2) {
      material[0] = 0.0; material[1] = color; material[2] = 0.0;
    }else if (velocidad == 3) {
      material[0] = color; material[1] = 0.0; material[2] = 0.0;
    }else if (velocidad == 4) {
      material[0] = color; material[1] = 0.0; material[2] = 0.0;
    }
    
    glEnable(GL_LIGHTING);  glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glMaterialfv(GL_FRONT, GL_AMBIENT, material);
      glTranslatef(0.0, 0.0, 60.0);
      glRotatef(90.0, 1.0, 0.0, 0.0);
      glColor3ub(0,255,0);
      glutSolidSphere(radius, 100, 100);
    glDisable(GL_DEPTH_TEST);
  }
}

// ======== draw multimarca =========================================
static void draw_multi( void ) {
  double  gl_para[16];   // Esta matriz 4x4 es la usada por OpenGL
  GLfloat material[]        = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_position[]  = {100.0,-200.0,200.0,0.0};
  int i;
  int respawn_flag = 0;
  
  argDrawMode3D();              // Cambiamos el contexto a 3D
  argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
  glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  argConvGlpara(mMarker->trans, gl_para);   
  glMatrixMode(GL_MODELVIEW);           
  glLoadMatrixd(gl_para);               

  glEnable(GL_LIGHTING);  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Renderizado del tablero y enemigos
  for (i = 0; i < mMarker->marker_num; i++){
    // No se ha detectado la marca
    if (mMarker->marker[i].visible < 0) {  
      // Si el enemigo estaba en el tablero y ahora no se ve, entonces lo hemos eliminado
      if (state_enemies[i] == 1) {
        state_enemies[i] = 0; // quitamos al enemigo del tablero y aumentamos los puntos
        puntuacion++;
        current_enemies--;
        printf("Puntuación: %d\n", puntuacion);
      }
      // Como no vemos la marca, se dibuja un cubo rojo
      material[0] = 1.0; material[1] = 0.0; material[2] = 0.0;
    }
    // Se ha detectado la marca
    else {
      // Si no tenemos el máximo número de enemigos en tablero, 
      // probamos a generar uno aleatoriamente dentro del tiempo de respawn
      if (current_enemies < max_enemies && state_enemies[i] == 0 
            && (game_time - last_respawn_time) >= time_to_respawn)
        spawn_enemies(i);
      
      // Si el enemigo está añadido al tablero, se dibuja un cubo blanco
      if (state_enemies[i] == 1) {
        material[0] = 0.0; material[1] = 0.0; material[2] = 0.0;
      }
      // Si no se ha añadido al tablero, se dibuja un cubo verde
      else {
        material[0] = 0.0; material[1] = 1.0; material[2] = 0.0;
      }
    }
    
    // Renderizamos los cubos en blanco, verde o rojo
    glPushMatrix();   // Guardamos la matriz actual
    argConvGlpara(mMarker->marker[i].trans, gl_para);   
    glMultMatrixd(gl_para); 
    glMaterialfv(GL_FRONT, GL_AMBIENT, material);
    glTranslatef(0.0, 0.0, 25.0);
    glutSolidCube(50.0);
    glPopMatrix();   // Recuperamos la matriz anterior
  }

  // DISTANCIAS
  double m[3][4], m2[3][4];
  for(int i = 0; i < mMarker->marker_num; i++) {
    if (mMarker->marker[i].visible < 0) { // no se detecta la marca
      //printf("Marca [%d] no detectada, estado previo %d\n", i, mMarker->marker[i].visibleR);
    }
    else if(mMarker->marker[i].visible == 1 && objects[0].visible == 1) {  // se ha detectado y bomba visible
      arUtilMatInv(objects[0].patt_trans, m);
      //arUtilMatMul(m, mMarker->marker[i].trans, m2);
      arUtilMatMul(m, mMarker->trans, m2);
      distancia = sqrt(pow(m2[0][3],2) + pow(m2[1][3],2) + pow(m2[2][3],2));
      printf ("Distancia bomba y enemigo multimarca[%d] = %G\n", i, distancia);
    }
  }

  // Calculámos el módulo del tiempo de partida, para generar enemigos cada X tiempo
  // if( fmod(game_time, (double) time_to_respawn) == 0.000000){
  //   printf("Timer reset %d\n", game_time);
  //   // Generar nuevos enemigos
  //   //arUtilTimerReset();
  // }

  glDisable(GL_DEPTH_TEST);
}

// ======== rotacion patron =========================================
static void cambiar_velocidad( void ) {
  // Modificar velocidad y nivel del juego
  if (velocidad == 1) {
    max_enemies = 2;
    time_to_respawn = 5;
  } else if (velocidad == 2) {
    max_enemies = 3;
    time_to_respawn = 5;
  } else if (velocidad == 3) {
    max_enemies = 4;
    time_to_respawn = 5;
  } else if (velocidad == 4) {
    max_enemies = 6;
    time_to_respawn = 5;
  }
}

static void calcular_velocidad( void ) {
  double  gl_para[16];   // Esta matriz 4x4 es la usada por OpenGL
  GLfloat light_position[]  = {100.0,-200.0,200.0,0.0};
  double v[3];
  float angle = 0.0, module = 0.0;
  double m[3][4], m2[3][4];
  argDrawMode3D();              // Cambiamos el contexto a 3D
  argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
  glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Al ser visible el objeto COIN
  argConvGlpara(objects[1].patt_trans, gl_para);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(gl_para);   // Cargamos su matriz de transf.

  glEnable(GL_LIGHTING);  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Calculamos el angulo de rotacion de la segunda marca si modo es 0, sino se queda el mismo color
  v[0] = objects[1].patt_trans[0][0];
  v[1] = objects[1].patt_trans[1][0];
  v[2] = objects[1].patt_trans[2][0];

  module = sqrt(pow(v[0],2) + pow(v[1],2) + pow(v[2],2));
  v[0] = v[0]/module;
  v[1] = v[1]/module;
  v[2] = v[2]/module;

  angle = acos (v[0]) * 57.2958;   // Sexagesimales! * (180/PI)
  angle = angle * 2;//de 180 grados pasamos a tener 360 grados

  if (angle <= 90) {
    velocidad = 1;
  } else if (angle > 90 && angle <= 180) {
    velocidad = 2;
  } else if (angle > 180 && angle <= 270) {
    velocidad = 3;
  } else if (angle > 270 && angle <= 360) {
    velocidad = 4;
  }
  
  // Modificamos la configuración del juego
  cambiar_velocidad();

  // Una vez tenemos la velocidad calculada y el ángulo, 
  // dibujamos la esfera de color en función de la velocidad y tamaño en función del ángulo
  draw_sphere(angle);

  if(velocidad != 0)
    printf("VELOCIDAD %d: Ángulo de %fº en el objeto coin\n", velocidad, angle);

}

// Delay function
void delay(int number_of_seconds) { 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds); 
}
// ======== init ====================================================
static void init( void ) {
  ARParam  wparam, cparam;   // Parametros intrinsecos de la camara
  int xsize, ysize;          // Tamano del video de camara (pixels)
  double c[2] = {0.0, 0.0};  // Centro de patron (por defecto)

  // Abrimos dispositivo de video
  if(arVideoOpen("") < 0) exit(0);  
  if(arVideoInqSize(&xsize, &ysize) < 0) exit(0);

  // Cargamos los parametros intrinsecos de la camara, tras haber calibrado la webcam
  if(arParamLoad("data/webcam.dat", 1, &wparam) < 0)   
    print_error ("Error en carga de parametros de camara\n");
  

  arParamChangeSize(&wparam, xsize, ysize, &cparam);
  arInitCparam(&cparam);   // Inicializamos la camara con "cparam"

  // Inicializamos la lista de objetos
  addObject("data/bomb.patt", 90.0, c, NULL);
  addObject("data/coin.patt", 90.0, c, NULL);
  
  // Cargamos el fichero de especificacion multimarca
  if( (mMarker = arMultiReadConfigFile("data/marker.dat")) == NULL )
    print_error("Error en fichero marker.dat\n");

  arUtilTimerReset();      // Reseteamos el tiempo de la partida a 0
   
  argInit(&cparam, 1.0, 0, 0, 0, 0);   // Abrimos la ventana 
}

// ======== mainLoop ================================================
static void mainLoop(void) {
  ARUint8 *dataPtr;
  ARMarkerInfo *marker_info;
  int marker_num, i, j, k;

  // Capturamos un frame de la camara de video
  if((dataPtr = (ARUint8 *)arVideoGetImage()) == NULL) {
    // Si devuelve NULL es porque no hay un nuevo frame listo
    arUtilSleep(2);  return;  // Dormimos el hilo 2ms y salimos
  }

  argDrawMode2D();
  argDispImage(dataPtr, 0,0);    // Dibujamos lo que ve la camara 

  // Detectamos la marca en el frame capturado (return -1 si error)
  if(arDetectMarker(dataPtr, 100, &marker_info, &marker_num) < 0) {
    cleanup(); exit(0);   // Si devolvio -1, salimos del programa!
  }

  arVideoCapNext();      // Frame pintado y analizado... A por otro!

  // Vemos donde detecta el patron con mayor fiabilidad
  for (i = 0; i < nobjects; i++) {
    for (j = 0, k = -1; j < marker_num; j++) {
      if (objects[i].id == marker_info[j].id) {
        if (k == -1) k = j;
        else if (marker_info[k].cf < marker_info[j].cf) k = j;
      }
    }

    if (k != -1) {   // Si ha detectado el patron en algun sitio...
      objects[i].visible = 1;
      // Obtener transformacion relativa entre la marca y la camara real
      arGetTransMat(&marker_info[k], objects[i].center, objects[i].width, objects[i].patt_trans);
      // Dibujar los objetos de la escena
      if (i == 0) {
        draw();       // Dibujamos sobre la bomba
      } else if (i == 1) { 
        // Detectar rotación para calcular velocidad, y dibujar el objeto
        calcular_velocidad(); 
      }

    } else {
      objects[i].visible = 0; // El objeto no es visible
    }
  }

  // Detectar patrón Multimarca
  if(arMultiGetTransMat(marker_info, marker_num, mMarker) > 0) 
    draw_multi();       // Dibujamos los objetos de la escena

  game_time = arUtilTimer();
  printf("Duración de la partida: %f segundos\n", game_time); // Imprimimos el tiempo de la partida

  argSwapBuffers(); // Cambiamos el buffer con lo que tenga dibujado
}

// ======== Main ====================================================
int main(int argc, char **argv) {
  glutInit(&argc, argv);    // Creamos la ventana OpenGL con Glut
  init();                   // Llamada a nuestra funcion de inicio
  arVideoCapStart();        // Creamos un hilo para captura de video
  argMainLoop( NULL, keyboard, mainLoop );   // Asociamos callbacks para las funciones de teclado
  return (0);
}
