
#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"

#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLBuffer.h"
#include "GL/GLVertexArray.h"
#include "ProgramManager.h"

#include <vector>

//! classe utilitaire : permet de construire une chaine de caracteres formatee. cf sprintf.
struct Format
{
    char text[1024];
    
    Format( const char *_format, ... )
    {
        text[0]= 0;     // chaine vide
        
        // recupere la liste d'arguments supplementaires
        va_list args;
        va_start(args, _format);
        vsnprintf(text, sizeof(text), _format, args);
        va_end(args);
    }
    
    ~Format( ) {}
    
    // conversion implicite de l'objet en chaine de caracteres stantard
    operator const char *( )
    {
        return text;
    }
};

struct TPObjet{

    gk::GLProgram *m_program;
    gk::GLVertexArray *m_vao;
    gk::GLBuffer *m_vertex_buffer;
    gk::GLBuffer *m_index_buffer;
    int m_indices_size;
    gk::Transform T;
    gk::Transform MV;
    gk::Transform NorM;
    gk::Mesh *mesh;

    void init(gk::Mesh *mesh,gk::GLProgram *m_program){
        m_vao= gk::createVertexArray();
        // cree le buffer de position
        m_vertex_buffer= gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
        // associe le contenu du buffer a la variable 'position' du shader
        glVertexAttribPointer(m_program->attribute("MCVertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
        // active l'utilisation du buffer
        glEnableVertexAttribArray(m_program->attribute("MCVertex"));
        // cree le buffer d'indices et l'associe au vertex array
        m_index_buffer= gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
        // conserve le nombre d'indices (necessaire pour utiliser glDrawElements)
        m_indices_size= mesh->indices.size();
    }

    void aurepere(gk::Transform X){
        T=X;
    }

    void draw(){
        // dessiner quelquechose
        glUseProgram(m_program->name);

        // parametrer le shader
        m_program->uniform("MVPMatrix")= T.matrix();      // transformation model view projection
        m_program->uniform("MVMatrix")= MV.matrix(); 
        m_program->uniform("NormalMatrix")= NorM.matrix();  
        //m_program->uniform("diffuse_color")= gk::VecColor(1, 1, 0);     // couleur des fragments

        // selectionner un ensemble de buffers et d'attributs de sommets
        glBindVertexArray(m_vao->name);
        // dessiner un maillage indexe
        glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, 0);

    }

};





//! squelette d'application gKit.
class TP : public gk::App
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program;

    gk::GLVertexArray *m_vao;
    
    gk::GLBuffer *m_vertex_buffer;
    gk::GLBuffer *m_index_buffer;
    int m_indices_size;
    
    gk::GLCounter *m_time;

    float width,height;
    float angle,znear,zfar;
    gk::Transform W ;
    gk::Transform P;
    gk::Transform V;
    gk::Transform T;
    gk::Transform Rx;
    gk::Transform Ry;
    gk::Transform S; //translate
    
    std::vector<TPObjet> objects;

public:
    // creation du contexte openGL et d'une fenetre
    TP( ):gk::App()
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        width=512;
        height=512;
        // cree le contexte et une fenetre
        if(createWindow(width,height, settings) < 0)
            closeWindow();
        
        m_widgets.init();
        m_widgets.reshape(windowWidth(), windowHeight());
    }
    
    ~TP( ) {}
    
    int init( )
    {
        // compilation simplifiee d'un shader program
        gk::programPath("shaders");
        m_program= gk::createProgram("leslie.glsl");
        if(m_program == gk::GLProgram::null())
            return -1;
        
        // charge un mesh
        gk::Mesh *mesh= gk::MeshIO::readOBJ("bigguy.obj");
        if(mesh == NULL)
            return -1;
        
        angle=45,znear=1,zfar=512;
        W = gk::Perspective (angle, 1, 1, 512);
        //P = gk::Viewport (width,height);
        V = gk::LookAt(gk::Point(0,0,50),gk::Point(0,0,0),gk::Vector(0,1,0));
        T=W*V;

        // cree le vertex array objet, description des attributs / associations aux variables du shader

        TPObjet bigguy;
        objects.push_back(bigguy);

        for(std::vector<TPObjet>::size_type i = 0; i != objects.size(); i++) {
            objects[i].init(mesh,m_program);
        }

        //////////// pour chaque objet
        /*m_vao= gk::createVertexArray();
        // cree le buffer de position
        m_vertex_buffer= gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
        // associe le contenu du buffer a la variable 'position' du shader
        glVertexAttribPointer(m_program->attribute("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
        // active l'utilisation du buffer 
        glEnableVertexAttribArray(m_program->attribute("position"));
        
        // cree le buffer d'indices et l'associe au vertex array
        m_index_buffer= gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
        // conserve le nombre d'indices (necessaire pour utiliser glDrawElements)
        m_indices_size= mesh->indices.size();*/
        //////////////
        
        // mesh n'est plus necessaire, les donnees sont transferees dans les buffers sur la carte graphique
        delete mesh;
        
        // nettoyage de l'etat opengl
        glBindVertexArray(0);   // desactive le vertex array
        glBindBuffer(GL_ARRAY_BUFFER, 0);       // desactive le buffer de positions
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);       // desactive le buffer d'indices
        
        // mesure du temps de dessin
        m_time= gk::createTimer();
        
        // ok, tout c'est bien passe
        return 0;
    }
    
    int quit( )
    {
        return 0;
    }

    // a redefinir pour utiliser les widgets.
    void processWindowResize( SDL_WindowEvent& event )
    {
        m_widgets.reshape(event.data1, event.data2);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseButtonEvent( SDL_MouseButtonEvent& event )
    {
        m_widgets.processMouseButtonEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseMotionEvent( SDL_MouseMotionEvent& event )
    {
        m_widgets.processMouseMotionEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processKeyboardEvent( SDL_KeyboardEvent& event )
    {
        m_widgets.processKeyboardEvent(event);
    }
    
    void keys(){
        if(key(SDLK_ESCAPE))
            // fermer l'application si l'utilisateur appuie sur ESCAPE
            closeWindow();

        if(key('r'))
        {
            key('r')= 0;
            // recharge et recompile les shaders
            gk::reloadPrograms();
        }

        if(key('c'))
        {
            key('c')= 0;
            // enregistre l'image opengl
            gk::writeFramebuffer("screenshot.png");
        }

        if(key(SDLK_LEFT)){
          Ry=gk::RotateY(-5);
          T=T*Ry;
        }
        if(key(SDLK_RIGHT)){
           Ry=gk::RotateY(5);
           T=T*Ry;
        }
        if(key(SDLK_UP)){
           Rx=gk::RotateX(5);
           T=T*Rx;
        }
        if(key(SDLK_DOWN)){
           Rx=gk::RotateX(-5);
           T=T*Rx;
        }
    }


    int draw( )
    {
        keys();
        //
        glViewport(0, 0, windowWidth(), windowHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // mesurer le temps d'execution
        m_time->start();
        
        // dessiner quelquechose
        /*glUseProgram(m_program->name);
        
        // parametrer le shader
        m_program->uniform("mvpMatrix")= T.matrix();      // transformation model view projection
        m_program->uniform("diffuse_color")= gk::VecColor(1, 1, 0);     // couleur des fragments
        
        // selectionner un ensemble de buffers et d'attributs de sommets
        glBindVertexArray(m_vao->name);
        // dessiner un maillage indexe
        glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, 0);*/
        
        // nettoyage
        glUseProgram(0);
        glBindVertexArray(0);
        
        // mesurer le temps d'execution
        m_time->stop();
        
        // afficher le temps d'execution
        {
            m_widgets.begin();
            m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);
            
            m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());
            
            m_widgets.endGroup();
            m_widgets.end();
        }
        
        // afficher le dessin
        present();
        // continuer
        return 1;
    }
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}

