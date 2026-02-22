struct Light {
  float3 direction;
  float3 ambient_color;
  float3 diffuse_colour;
  float3 specular_colour;
};

float4 calculate_pbr(float4 albedo, float3 normals, float3 frag_pos, float3 camera_pos, Light light) {
  // ambient
  float ambient_intensity = 0.2;
  float3 ambient = ambient_intensity * light.ambient_color;

  // diffuse
  float3 light_dir = normalize(light.direction - frag_pos);
  float diffuse_intensity = max(dot(normals, light_dir), 0.0);
  if (diffuse_intensity >= 0.8)
      diffuse_intensity = 1.0;
  else if (diffuse_intensity >= 0.6)
      diffuse_intensity = 0.6;
  else if (diffuse_intensity >= 0.3)
      diffuse_intensity = 0.3;
  else
      diffuse_intensity = 0.0;
  float3 diffuse = diffuse_intensity * light.diffuse_colour;

  // specular
  float specularStrength = 0.25;
  float3 viewDir = normalize(camera_pos - frag_pos);
  float3 reflectDir = reflect(-light_dir, normals);
  float specular_intensity = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  specular_intensity = step(0.98, specular_intensity);
  float3 specular = specularStrength * specular_intensity * light.specular_colour;

  float3 result = ambient + diffuse + specular;
  return albedo * float4(result, 1.0);
}
