/*!
 * @file 
 * @brief This file contains implemenation of phong vertex and fragment shader.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include<math.h>
#include<assert.h>

#include"student/student_shader.h"
#include"student/gpu.h"
#include"student/uniforms.h"

#include <stdio.h>

/// \addtogroup shader_side Úkoly v shaderech
/// @{

void phong_vertexShader(
	GPUVertexShaderOutput     *const output,
	GPUVertexShaderInput const*const input ,
	GPU                        const gpu   ){
  /// \todo Naimplementujte vertex shader, který transformuje vstupní vrcholy do clip-space.<br>
  /// <b>Vstupy:</b><br>
  /// Vstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu ve world-space (vec3) a v prvním
  /// atributu obsahovat normálu vrcholu ve world-space (vec3).<br>
  /// <b>Výstupy:</b><br>
  /// Výstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu (vec3) ve world-space a v prvním
  /// atributu obsahovat normálu vrcholu ve world-space (vec3).
  /// Výstupní vrchol obsahuje pozici a normálu vrcholu proto, že chceme počítat osvětlení ve world-space ve fragment shaderu.<br>
  /// <b>Uniformy:</b><br>
  /// Vertex shader by měl pro transformaci využít uniformní proměnné obsahující view a projekční matici.
  /// View matici čtěte z uniformní proměnné "viewMatrix" a projekční matici čtěte z uniformní proměnné "projectionMatrix".
  /// Zachovejte jména uniformních proměnných a pozice vstupních a výstupních atributů.
  /// Pokud tak neučiníte, akceptační testy selžou.<br>
  /// <br>
  /// Využijte vektorové a maticové funkce.
  /// Nepředávajte si data do shaderu pomocí globálních proměnných.
  /// Pro získání dat atributů použijte příslušné funkce vs_interpret* definované v souboru program.h.
  /// Pro získání dat uniformních proměnných použijte příslušné funkce shader_interpretUniform* definované v souboru program.h.
  /// Vrchol v clip-space by měl být zapsán do proměnné gl_Position ve výstupní struktuře.<br>
  /// <b>Seznam funkcí, které jistě použijete</b>:
  ///  - gpu_getUniformsHandle()
  ///  - getUniformLocation()
  ///  - shader_interpretUniformAsMat4()
  ///  - vs_interpretInputVertexAttributeAsVec3()
  ///  - vs_interpretOutputVertexAttributeAsVec3()

  	//get handle to all uniforms
	Uniforms const uniformsHandle = gpu_getUniformsHandle(gpu);
  
  	//get uniform location of view matrix
	UniformLocation const viewMatrixLocation = getUniformLocation(gpu, "viewMatrix");
  	//get pointer to view matrix
	Mat4 const*const view = shader_interpretUniformAsMat4(uniformsHandle, viewMatrixLocation);
  	//get uniform location of projection matrix
	UniformLocation const projectionMatrixLocation = getUniformLocation(gpu, "projectionMatrix");
  	//get pointer to projection matrix
	Mat4 const*const proj = shader_interpretUniformAsMat4(uniformsHandle, projectionMatrixLocation);

	Vec3 const*const position = vs_interpretInputVertexAttributeAsVec3(gpu, input, 0);

	Mat4 mvp;
	multiply_Mat4_Mat4(&mvp, proj, view);
  
  	Vec4 pos4;
  	copy_Vec3Float_To_Vec4(&pos4, position, 1.f);
  
  	multiply_Mat4_Vec4(&output->gl_Position, &mvp, &pos4);

	Vec3 *const color = vs_interpretOutputVertexAttributeAsVec3(gpu , output, 0);
	init_Vec3(color, (float)(input->gl_VertexID == 0), (float)(input->gl_VertexID == 1), (float)(input->gl_VertexID == 2));

	Vec3 *const positionAttr = vs_interpretOutputVertexAttributeAsVec3(gpu , output, 0);
	init_Vec3(positionAttr, position->data[0], position->data[1], position->data[2]);

	Vec3 const*const normal = vs_interpretInputVertexAttributeAsVec3(gpu, input, 1);
	
	Vec3 *const normalAttr = vs_interpretOutputVertexAttributeAsVec3(gpu, output, 1);
	init_Vec3(normalAttr, normal->data[0], normal->data[1], normal->data[2]);
	
  // (void)output;
  // (void)input;
  // (void)gpu;
}

void phong_fragmentShader(
	GPUFragmentShaderOutput     *const output,
	GPUFragmentShaderInput const*const input ,
	GPU                          const gpu   ){
  /// \todo Naimplementujte fragment shader, který počítá phongův osvětlovací model s phongovým stínováním.<br>
  /// <b>Vstup:</b><br>
  /// Vstupní fragment by měl v nultém fragment atributu obsahovat interpolovanou pozici ve world-space a v prvním
  /// fragment atributu obsahovat interpolovanou normálu ve world-space.<br>
  /// <b>Výstup:</b><br> 
  /// Barvu zapište do proměnné color ve výstupní struktuře.<br>
  /// <b>Uniformy:</b><br>
  /// Pozici kamery přečtěte z uniformní proměnné "cameraPosition" a pozici světla přečtěte z uniformní proměnné "lightPosition".
  /// Zachovejte jména uniformních proměnný.
  /// Pokud tak neučiníte, akceptační testy selžou.<br>
  /// <br>
  /// Dejte si pozor na velikost normálového vektoru, při lineární interpolaci v rasterizaci může dojít ke zkrácení.
  /// Zapište barvu do proměnné color ve výstupní struktuře.
  /// Shininess faktor nastavte na 40.f
  /// Difuzní barvu materiálu nastavte na čistou zelenou.
  /// Spekulární barvu materiálu nastavte na čistou bílou.
  /// Barvu světla nastavte na bílou.
  /// Nepoužívejte ambientní světlo.<br>
  /// <b>Seznam funkcí, které jistě využijete</b>:
  ///  - shader_interpretUniformAsVec3()
  ///  - fs_interpretInputAttributeAsVec3()

	// create uniforms handle
	Uniforms const uniformsHandle = gpu_getUniformsHandle(gpu);

	// CAMERA POSITION
	UniformLocation const cameraPositionUniform = getUniformLocation(gpu, "cameraPosition");
	Vec3 const* cameraPosition = shader_interpretUniformAsVec3(uniformsHandle, cameraPositionUniform);	
		
	// LIGHTS POSITION
	UniformLocation const lightPositionLocation = getUniformLocation(gpu, "lightPosition");
	Vec3 const* lightsPosition = shader_interpretUniformAsVec3(uniformsHandle, lightPositionLocation);	

	output->depth = input->depth;

	// get position and normal from attributes
	Vec3 const* interpolatedPosition = fs_interpretInputAttributeAsVec3(gpu, input, 0);//index of fragment attribute
	Vec3 const* interpolatedNormal = fs_interpretInputAttributeAsVec3(gpu, input, 1);

	// normalize the normal
	Vec3 normalizedNormal; 
	normalize_Vec3(&normalizedNormal, interpolatedNormal); //[0,1,0]

	// vector from surface point to light source
	Vec3 surfaceToSource; //[0,1,0]
	sub_Vec3(&surfaceToSource, lightsPosition, interpolatedPosition); //[0, 100, 0], [10, 10, 10]
	normalize_Vec3(&surfaceToSource, &surfaceToSource);
	
	// vector from surface point to camera
	Vec3 surfaceToCamera;
	sub_Vec3(&surfaceToCamera, interpolatedPosition, cameraPosition); //[0, 20, 20], [10, 10, 10]
	normalize_Vec3(&surfaceToCamera, &surfaceToCamera);

	// direction that a perfectly reflected ray of light would take from this point on the surface
	Vec3 reflection;
	reflect(&reflection, &surfaceToSource, &normalizedNormal);

	float shininess = 40.f;
	
	// diffuse reflection constant
	float diffuseCoeficient = (float)(dot_Vec3(&normalizedNormal, &surfaceToSource) < 0 ? 0.0 : dot_Vec3(&normalizedNormal, &surfaceToSource));
	Vec3 diffuseColor; init_Vec3(&diffuseColor, 0.0, diffuseCoeficient, 0.0);

	// specular reflection constant
	float specularCoefficient = 0.0;
	if (diffuseCoeficient > 0) {
		if (dot_Vec3(&surfaceToCamera, &reflection) > 0) {
			specularCoefficient = powf(dot_Vec3(&reflection, &surfaceToCamera), shininess);
		}
	}
	Vec3 specularColor; init_Vec3(&specularColor, specularCoefficient, specularCoefficient, specularCoefficient);

	Vec3 medzi; add_Vec3(&medzi, &specularColor, &diffuseColor);
	copy_Vec3Float_To_Vec4(&output->color, &medzi, 1.f); 
	
  // (void)output;
  // (void)input;
  // (void)gpu;

}

/// @}
