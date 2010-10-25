// http://www.gamedev.net/community/forums/topic.asp?topic_id=372739


//  interpolate these and pass 
//  them into the fragment shader
varying vec2 texCoord;
varying vec3 vertex_pos;
varying float vertex_dist;
varying vec3 normal;
varying vec3 lightDir;
varying vec3 halfVec;

void main(void)
{
   // location of the vertex in eye space
   vec3 eyeSpaceVert = (gl_ModelViewMatrix * gl_Vertex).xyz;
   
   vec3 eyeSpaceLightDir = (gl_ModelViewMatrix * vec4(-0.707, -0.707, -0.707, 0.0)).xyz;
   
   vec3 eyeSpaceHalfVec = normalize(eyeSpaceLightDir + vec3(0, 0, -1)); // Only comes out 0 if looking at backfaces, so shouldn't matter I hope


   
   //  the matrix needed to convert to eye space
   //  (this is local, and should already be normalized, I think)
   vec3 eyeSpaceTangent  = gl_NormalMatrix * gl_MultiTexCoord1.xyz;	//tangent;
   vec3 eyeSpaceBinormal = gl_NormalMatrix * gl_MultiTexCoord2.xyz;	//binormal;
   vec3 eyeSpaceNormal   = gl_NormalMatrix * gl_Normal; //normal
   
   normal = eyeSpaceNormal;
  
   // now convert the light and position, and pass in the texture coordinate
   vertex_pos = vec3 (
        dot (eyeSpaceTangent, eyeSpaceVert),
        dot (eyeSpaceBinormal, eyeSpaceVert),
        dot (eyeSpaceNormal, eyeSpaceVert));
   lightDir = normalize(vec3 (
        dot (eyeSpaceTangent, eyeSpaceLightDir),
        dot (eyeSpaceBinormal, eyeSpaceLightDir),
        dot (eyeSpaceNormal, eyeSpaceLightDir)));
   halfVec = normalize(vec3 (
        dot (eyeSpaceTangent, eyeSpaceHalfVec),
        dot (eyeSpaceBinormal, eyeSpaceHalfVec),
        dot (eyeSpaceNormal, eyeSpaceHalfVec)));
  
   vertex_dist = length (eyeSpaceVert);
   texCoord   = gl_MultiTexCoord0.xy;
   
   // done
   gl_Position = ftransform();
}
