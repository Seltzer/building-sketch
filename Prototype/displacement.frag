// http://www.gamedev.net/community/forums/topic.asp?topic_id=372739

varying vec2 texCoord;
varying vec3 vertex_pos;
varying float vertex_dist;
varying vec3 normal;

//uniform sampler2D stepmap;
//uniform sampler2D texmap;

void main()
{
	float bright = dot(normal, vec3(0, 0, 1));
	gl_FragColor = vec4(bright, bright, bright, 1);
	//gl_FragColor = vec4(texCoord.xy, 0, 1);
}
