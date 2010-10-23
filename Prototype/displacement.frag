// http://www.gamedev.net/community/forums/topic.asp?topic_id=372739

varying vec2 texCoord;
varying vec3 vertex_pos;
varying float vertex_dist;
varying vec3 normal;
varying vec3 lightDir;
varying vec3 halfVec;

uniform sampler2D reliefmap;
uniform sampler2D normalmap;
//uniform sampler2D texmap;

void ray_intersect_rm(inout vec3 dp, in vec3 ds);

void main()
{
	//float bright = dot(normal, vec3(0, 0, 1));
	//gl_FragColor = vec4(bright, bright, bright, 1);
	//gl_FragColor = vec4(texCoord.xy, 0, 1);
	//gl_FragColor = texture2D(stepmap, texCoord) * bright;
	//gl_FragColor = texture2D(normalmap, texCoord);
	//return;
	
	float depth = 0.05; // Adjust scale of displacement
	
	// ray intersect in view direction
	float a = -depth / vertex_pos.z;
	vec3 s = vertex_pos * a;
	s.z = 1.0;

	//	find the distance to the heightfield
	vec3 pt_eye = vec3(texCoord, 0.0);
	ray_intersect_rm(pt_eye, s);

	// look up surface color (TODO: Use color texture)
	//vec4 color = texture2D(reliefmap, pt_eye.xy);
	vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
	
	// expand normal from normal map in local polygon space
	// blue = df/dx
	// alpha = df/dy (image coords are no longer backward!)
	// note: I _need_ the texture size to scale the normal properly!
	// Does this ruin things if the texture is stretched?... which it is.
	// Maybe better to use a real normal map
	vec4 normalLookup = texture2D(normalmap, pt_eye.xy);
	vec3 normal = normalLookup.xyz - 0.5;
	//vec4 normalLookup = texture2D(normalmap, pt_eye.xy);
	//vec3 normal = vec3((0.5-normalLookup.ba) * (-depth * 255.0), 1.0);
	normal = normalize(normal);
	
	float ambient = 0.25;
	float diffuse = 0.6 * max(0.0, -dot(lightDir, normal));
	float specular = 0.2 * pow(max(0.0, -dot(normal, halfVec)), 20);
	gl_FragColor = vec4(color.xyz * (ambient + diffuse) + specular, 1.0);
	//gl_FragColor = texture2D(normalmap, pt_eye.xy);
}

//  vanilla Relief Mapping
void ray_intersect_rm(inout vec3 dp, in vec3 ds)
{
	int linear_search_steps = 10;
	int binary_search_steps = 10;

	float depth_step=1.0/float(linear_search_steps);

	//  linear steps
	for (int i = 0; i < linear_search_steps; ++i)
	{
		vec4 t=1.0-texture2D(reliefmap,dp.xy);
		dp += ds * depth_step * step (dp.z, t.r);
	}

	//  binary search
	for (int i = 0; i < binary_search_steps; ++i)
	{
		vec4 t=1.0-texture2D(reliefmap,dp.xy);
		dp += ds * depth_step * (step (dp.z, t.r) - 0.5);
		depth_step *= 0.5;
	}  

	// all done
	return;
}
