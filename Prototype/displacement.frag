// http://www.gamedev.net/community/forums/topic.asp?topic_id=372739

varying vec2 texCoord;
varying vec3 vertex_pos;
varying float vertex_dist;
varying vec3 normal;

uniform sampler2D reliefmap;
//uniform sampler2D texmap;

void ray_intersect_rm(inout vec3 dp, in vec3 ds);

void main()
{
	//float bright = dot(normal, vec3(0, 0, 1));
	//gl_FragColor = vec4(bright, bright, bright, 1);
	//gl_FragColor = vec4(texCoord.xy, 0, 1);
	//gl_FragColor = texture2D(stepmap, texCoord) * bright;
	
	
	float depth = 0.05; // Adjust scale of displacement
	
	// ray intersect in view direction
	float a = -depth / vertex_pos.z;
	vec3 s = vertex_pos * a;
	s.z = 1.0;

	//	find the distance to the heightfield
	vec3 pt_eye = vec3(texCoord, 0.0);
	ray_intersect_rm(pt_eye, s);

	// compute the final color (TODO: Use color texture)
	gl_FragColor = texture2D(reliefmap, pt_eye.xy);
}

//  vanilla Relief Mapping
void ray_intersect_rm(inout vec3 dp, in vec3 ds)
{
	int linear_search_steps = 20;
	int binary_search_steps = 20;

	float depth_step=1.0/float(linear_search_steps);

	//  linear steps
	for (int i = 0; i < linear_search_steps; ++i)
	{
		vec4 t=texture2D(reliefmap,dp.xy);
		dp += ds * depth_step * step (dp.z, t.r);
	}

	//  binary search
	for (int i = 0; i < binary_search_steps; ++i)
	{
		vec4 t=texture2D(reliefmap,dp.xy);
		dp += ds * depth_step * (step (dp.z, t.r) - 0.5);
		depth_step *= 0.5;
	}  

	// all done
	return;
}
