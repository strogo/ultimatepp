#ifndef _SurfaceCtrl_Object3D_h_
#define _SurfaceCtrl_Object3D_h_
#include "Definition.h"
#include "BoundingBox.h"
#include "Mesh.h"
#include "Transform.h"

namespace Upp{
/*
struct Light{
	Light(){
		Position =vec3( 1.2, 80.0, 2.0);
		Ambient =vec3( 0.9, 0.9, 0.9);
		Diffuse =vec3( 0.5, 0.5, 0.5);
		Specular =vec3( 0.0, 1.0, 1.0);
	}
	Light(glm::vec3 _position, glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular){
		Position = _position;
	    Ambient  = _ambient;
	    Diffuse  = _diffuse;
	    Specular = _specular;
	}
	
	glm::vec3 Position;
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
};*/
struct Texture : public Moveable<Texture>{
	Upp::String name="";
	unsigned int id =0;
	
	String ToString()const{return "name=" + name + " | " + "id=" + AsString(id);}
	
	Texture& operator=(const Texture& t){
		name = t.name;
		id = t.id;
		return *this;
	}
	
};


class Object3D : public Upp::Moveable<Object3D>{
	private:
		static int GlobalID;
		int ID;

		Transform transform;
		BoundingBox boundingBox;
		Material material; //The material object is a representation of material property of the object (it change how light affect it)

		Vector<Mesh> meshes;
		Vector<Texture> textures; //Vector carrying all texture of the object, every meshes refer to it via one iterator
		Vector<OpenGLProgram> program;
		VectorMap<String,Value> objectValues; //In case data need to be used in Init() Clear() and Draw();

		bool loaded = false;
		bool moved = false;
		bool visible = true;
		bool showBoundingBox = false;

		int ProgramNoLight = 0;//The program will draw figure without light
		int ProgramLine =  0; //THe program will draw figure line
		int ProgramNormal =  0;//The program will draw Normal
		int ProgramLight =  0;//the program will draw figure with light
		
		GLenum drawType = GL_TRIANGLES;
	public:
		Function <void()> WhenInit;
		Function <void(const glm::mat4& projectionMatrix,const glm::mat4& viewMatrix,const glm::vec3& viewPosition)> WhenDraw;
		Function <void()> WhenClear;
		Function <void()> WhenReload;
		
		/*
		Color lineColor = Black();
		float lineOpacity = 0.5f;
		float lineWidth = 1.0f;
		
		Color normalColor = Red();
		float normalOpacity = 0.5f;
		float normalLenght = 1.0f;
		
		OpenGLProgram NoLight;
		OpenGLProgram Line;
		OpenGLProgram Normal;
		OpenGLProgram Light;

		bool showMesh = true;
		bool showMeshLine = false;
		bool showMeshNormal = false;
		bool showLight = true;
		*/
	private:
		bool UpdateBuffer(GLuint buffer, int SurfaceCount , int SurfaceNumber,int count ,int numberOfElement, const float * data)noexcept;
		Vector<float> ReadBuffer(GLuint buffer, int SurfaceCount , int SurfaceNumber,int count, int numberOfElement)noexcept;
		//Texture loading function
		//Return position of the texture object carrying this texture
		int LoadTexture(const Image& img , const String& name, int indiceWhereInsert = -1);
		/*
			Assimp loading function
		*/
		bool InitFromScene(const aiScene* pScene, const String& Filename);
	    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
	    bool InitMaterials(const aiScene* pScene, const String& Filename);
	public:
		Object3D():ID(++GlobalID){
			objectValues.Add("showMesh", Value(true));
			objectValues.Add("showMeshLine", Value(false));
			objectValues.Add("showMeshNormal", Value(false));
			objectValues.Add("showLight", Value(true));
			
			objectValues.Add("lineColor", RawToValue(Black()));
			objectValues.Add("lineOpacity", Value(0.5f));
			objectValues.Add("lineWidth", Value(1.0f));
			
			objectValues.Add("normalColor", RawToValue(Red()));
			objectValues.Add("normalOpacity", Value(0.5f));
			objectValues.Add("normalLenght", Value(1.0f));
		}
		//move will prevent your object to be deleted (from OpenGL perspective)
		Object3D(Object3D&& obj):ID(GlobalID++){*this = pick(obj);}
		Object3D& operator=(Object3D&& obj);
		
		//copy also VBO and VAO , wich mean when object is destroyed , every copy will
		//lost their 3D representation in OpenGL
		Object3D(const Object3D& obj):ID(GlobalID++){*this = obj;}
		Object3D& operator=(const Object3D& obj);
		
		~Object3D();

		/**
			LoadModel load multiple kind of object file using Assimp lib, you can specify some custom
			load routine by editing pFlags, if pFlags = 0 then SurfaceCtrl default behavior about
			assimp will be used
		**/
		Object3D& LoadModel(const String& FileObj, Color color = Gray(), int alpha =255, unsigned int pFlags = 0);
		Object3D& LoadSurface(Surface& surface, Upp::Color = Green(), int alpha =255);
		Surface GetSurface();
		
		
		//Check if texture provided is already load, then return the iterator of the texture
		//Object3D
		int InsertTexture(const String& filename,int indice = -1, FlipMode flipmode = FLIP_NONE)noexcept; //insert texture in object3D
		int InsertTexture(const Image& m,int indice = -1, FlipMode flipmode = FLIP_NONE)noexcept; //insert texture in object3D
		int InsertTexture(const TexturesMaterial& tm,int indice = -1, FlipMode flipmode = FLIP_NONE)noexcept; //Insert one of SurfaceCtrl provided texture
		
		const Texture& GetTexture(int indice){if(indice < textures.GetCount()) return textures[indice];else throw Exc("int indice higher than textures.getCount()");}//Return the indice, Can throw exception

		Object3D& AttachTexture(int TexNo,int MeshNo, int NumberMeshToAffect =1);//Attach a texture to the range of mesh
		Object3D& GenerateTextureCoordinate(int MeshNo, int NumberMeshToAffect =1, bool CustomTextureCoordinate = false,const Vector<float>& tc = Vector<float>());// Generate new Texture coordinate for the selected range of mesh
		
		
		int GetID()const{return ID;}
		
		const Upp::Vector<Mesh>& GetMeshes() const noexcept{return meshes;}
		Mesh& CreateMesh()noexcept{return meshes.Add();}

		bool Init(); //Load all data in graphic memory It's called automaticly by using Load function, but you must call it if you set manually all data
		Object3D& Clear();
		Object3D& Reload(); //Unload and reload
		
		Object3D& ShowMesh(bool b = true)noexcept{if(objectValues.Find("showMesh") != -1) objectValues.Get("showMesh")= b; return *this;}
		Object3D& ShowMeshLine(bool b = true)noexcept{if(objectValues.Find("showMeshLine") != -1) objectValues.Get("showMeshLine")= b; return *this;}
		Object3D& ShowMeshNormal(bool b = true)noexcept{if(objectValues.Find("showMeshNormal") != -1) objectValues.Get("showMeshNormal")= b; return *this;}
		Object3D& ShowLight(bool b = true)noexcept{if(objectValues.Find("showLight") != -1) objectValues.Get("showLight") = b; return *this;}
		Object3D& ShowBoundingBox(bool b = true)noexcept{showBoundingBox = b; return *this;}
		
		bool GetShowMesh()const noexcept{if(objectValues.Find("showMesh") != -1) return objectValues.Get("showMesh"); else return false;}
		bool GetShowMeshLine()const noexcept{if(objectValues.Find("showMeshLine") != -1) return (bool)objectValues.Get("showMeshLine"); else return false;}
		bool GetShowMeshNormal()const noexcept{if(objectValues.Find("showMeshNormal") != -1) return (bool)objectValues.Get("showMeshNormal"); else return false;}
		bool GetShowLight()const noexcept{if(objectValues.Find("showLight") != -1) return (bool)objectValues.Get("showLight"); else return false;}
		bool GetShowBoundingBox()const noexcept{return showBoundingBox;}

		Object3D& SetLineColor(Color color)noexcept{if(objectValues.Find("lineColor") != -1) objectValues.Get("lineColor")= color; return *this;}
		Object3D& SetNormalColor(Color color)noexcept{if(objectValues.Find("normalColor") != -1) objectValues.Get("normalColor")= color; return *this;}
		Object3D& SetLineOpacity(float opacity)noexcept{if(opacity < 0) opacity =0; if(opacity> 1.0f) opacity = 1.0f; if(objectValues.Find("lineOpacity") != -1) objectValues.Get("lineOpacity") = opacity;  return *this;}
		Object3D& SetLineWidth(float width)noexcept{if(width < 1) width = 1.0f; if(objectValues.Find("lineWidth") != -1) objectValues.Get("lineWidth") = width; return *this;}
		Object3D& SetNormalOpacity(float opacity)noexcept{if(opacity < 0) opacity =0; if(opacity> 1.0f) opacity = 1.0f;if(objectValues.Find("normalOpacity") != -1) objectValues.Get("normalOpacity")= opacity; return *this;}
		
		const VectorMap<String,Value>& GetObjectsValues(){return objectValues;}
		
		Object3D& SetDrawType(GLenum drawtype)noexcept{drawType = drawtype;return *this;}
		GLenum GetDrawType()const noexcept{return drawType;}
		
		Upp::Color GetLineColor()const noexcept{if(objectValues.Find("lineColor") != -1) return objectValues.Get("lineColor").Get<Color>(); else return Black();}
		Upp::Color GetNormalColor()const noexcept{if(objectValues.Find("normalColor") != -1) return objectValues.Get("normalColor").Get<Color>(); else return Black();}
		float GetNormalOpcaity()const noexcept{if(objectValues.Find("normalOpacity") != -1) return (double)(objectValues.Get("normalOpacity")); else return 0.0f;}
		float GetLineOpcaity()const noexcept{if(objectValues.Find("lineOpacity") != -1) return (double)(objectValues.Get("lineOpacity")); else return 0.0f;}
		float GetLineWidth()const noexcept{if(objectValues.Find("lineWidth") != -1) return (double)(objectValues.Get("lineWidth")); else return 0.0f;}
				
		Transform& GetTransform()noexcept{return transform;}
		const Transform& GetTransform()const noexcept{return transform;}
				
		void CreateBoundingBox(); // Create Bounding box
		void RemoveBoundingBox();
		
		BoundingBox& GetBoundingBox(){return boundingBox;}
		BoundingBox GetBoundingBoxTransformed()const noexcept{
			BoundingBox box(boundingBox);
			return box.TransformBy(transform.GetModelMatrix());
		}
		
		bool TestLineIntersection(const glm::vec3 & start, const glm::vec3 & end)const{
			BoundingBox box  = Upp::pick(GetBoundingBoxTransformed());
			return box.LineIntersection(start,end);
		}
		
		Object3D& Show(){visible = true;return *this;}
		Object3D& Hide(){visible = false;return *this;}
		Object3D& SetVisible(bool b = true){visible = b; return *this;}
		bool& IsVisible(){return visible;}
		
		/*
			Write into OpenGL buffer return true if operation have been done correctly.
			/!\ True do not mean OpenGL will provide good things, it just mean buffer have been
			    writted  correctly
		*/
		bool UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, int r, int g, int b,int alpha)noexcept;
		bool UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, float r, float g, float b,float alpha)noexcept;
		bool UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 color,float alpha)noexcept;
		bool UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, Upp::Color color,int alpha)noexcept;
		bool UpdateColors(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept;
		
		bool UpdateNormal(unsigned int MeshNo, unsigned int SurfaceNumber, float x, float y, float z)noexcept;
		bool UpdateNormal(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 normal)noexcept;
		bool UpdateNormals(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept;
		
		bool UpdateVertice(unsigned int MeshNo, unsigned int SurfaceNumber, float x, float y, float z)noexcept;
		bool UpdateVertice(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 vertice)noexcept;
		bool UpdateVertices(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept;
		/*
			Read OpenGL buffer and return data in Upp::Vector
		*/
		Vector<float> ReadColors(int MeshNo, unsigned int SurfaceNumber, int count);
		Vector<float> ReadNormals(int MeshNo, unsigned int SurfaceNumber, int count);
		Vector<float> ReadVertices(int MeshNo, unsigned int SurfaceNumber, int count);
		
		Object3D& SetProgramNoLight(const OpenGLProgram& prog)noexcept{ProgramNoLight = program.GetCount(); program.Add(prog); return *this;}
		Object3D& SetProgramLight(const OpenGLProgram& prog)noexcept{ProgramLight = program.GetCount(); program.Add(prog); return *this;}
		Object3D& SetProgramLine(const OpenGLProgram& prog)noexcept{ProgramLine = program.GetCount(); program.Add(prog); return *this;}
		Object3D& SetProgramNormal(const OpenGLProgram& prog)noexcept{ProgramNormal = program.GetCount(); program.Add(prog); return *this;}
		
		void Draw(const glm::mat4& projectionMatrix,const glm::mat4& viewMatrix,const glm::vec3& viewPosition)noexcept;
};

class Skybox {
	private:
		unsigned int ID = 0;
		GLuint VBO, VAO;
		
		OpenGLProgram program;
		
	public:
		
		Skybox(){}
		~Skybox(){Clear();}
		
		Skybox& Init();
		Skybox& Clear();
		Skybox& Draw(const glm::mat4& projectionMatrix,const glm::mat4& viewMatrix);

};
	
}

#endif
