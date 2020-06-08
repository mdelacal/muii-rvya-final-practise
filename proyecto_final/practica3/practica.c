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

// ==== Definicion de constantes y variables globales ===============
int    patt_id;            // Identificador unico de la marca
double patt_trans[3][4];   // Matriz de transformacion de la marca

void print_error (char *error) {  printf(error); exit(0); }

// ======== cleanup =================================================
static void cleanup(void) {   
  arVideoCapStop();            // Libera recursos al salir ...
  arVideoClose();
  argCleanup();
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
void addObject(char *p, double w, double c[2], void (*drawme)(float,float,float))
{
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

// ======== draw ====================================================
static void draw( void ) {
  double  gl_para[16];   // Esta matriz 4x4 es la usada por OpenGL
  GLfloat mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
  GLfloat light_position[]  = {100.0,-200.0,200.0,0.0};
  
  /* Pintamos todos los objetos visibles */
  for (int i = 0; i < nobjects; i++) {
    if (objects[i].visible) {   // Si el objeto es visible
      argDrawMode3D();              // Cambiamos el contexto a 3D
      argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
      glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
      
      argConvGlpara(objects[i].patt_trans, gl_para);   // Convertimos la matriz de
      glMatrixMode(GL_MODELVIEW);                      // la marca para ser usada
      glLoadMatrixd(gl_para);                          // por OpenGL
      
      // Esta ultima parte del codigo es para dibujar el objeto 3D
      glEnable(GL_LIGHTING);  glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0, GL_POSITION, light_position);
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glTranslatef(0.0, 0.0, 60.0);
        glRotatef(90.0, 1.0, 0.0, 0.0);
        glutSolidTeapot(80.0);
      glDisable(GL_DEPTH_TEST);
    }
  }
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
  addObject("data/bomb.patt", 90.0, c, draw);
  addObject("data/coin.patt", 90.0, c, draw);

  argInit(&cparam, 1.0, 0, 0, 0, 0);   // Abrimos la ventana 
}

// ======== mainLoop ================================================
static void mainLoop(void) {
  ARUint8 *dataPtr;
  ARMarkerInfo *marker_info;
  int marker_num, i, j, k;

  double p_width     = 90.0;        // Ancho del patron (marca)
  // double p_width     = 120.0;        // Ancho del patron (marca)
  double p_center[2] = {0.0, 0.0};   // Centro del patron (marca)

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
    for(j = 0, k = -1; j < marker_num; j++) {
      if(objects[i].id == marker_info[j].id) {
        if (k == -1) k = j;
        else if(marker_info[k].cf < marker_info[j].cf) k = j;
      }
    }

    if(k != -1) {   // Si ha detectado el patron en algun sitio...
      objects[i].visible = 1;
      // Obtener transformacion relativa entre la marca y la camara real
      arGetTransMat(&marker_info[k], objects[i].center, objects[i].width, objects[i].patt_trans);
      draw();       // Dibujamos los objetos de la escena
    }else {
      objects[i].visible = 0; // El objeto no es visible
    }
  }

  argSwapBuffers(); // Cambiamos el buffer con lo que tenga dibujado
}

// ======== Main ====================================================
int main(int argc, char **argv) {
  glutInit(&argc, argv);    // Creamos la ventana OpenGL con Glut
  init();                   // Llamada a nuestra funcion de inicio
  arVideoCapStart();        // Creamos un hilo para captura de video
  argMainLoop( NULL, keyboard, mainLoop );   // Asociamos callbacks para las funciones de teclado

  printf("¡¡¡Bienvenido al juego de Super Mario!!!");

  return (0);
}
