struct VS_IN
{
	float3 p : POSITION;
	float4 c : COLOR;
};
struct VS_OUT
{
	float4 p : SV_POSITION;
	float4 c : COLOR0;
	float2 t : TEXCOORD;
};
cbuffer cb_data : register(b0)
{
	matrix g_matView : packoffset(c0);
	matrix g_matProj : packoffset(c4);
	matrix g_matComb : packoffset(c8);
	float4 g_vXAxis : packoffset(c12);
	float4 g_vYAxis : packoffset(c13);
	float4 g_vZAxis : packoffset(c14);
	float4 g_vSize : packoffset(c15);
};

VS_OUT VS(VS_IN input) 
{
	VS_OUT output = (VS_OUT)0;
	output.p = float4(input.p.xyz, 1.0f);	
	output.c = input.c;
	output.t = float2(0, 0);

	return output;
}
struct G_OUT
{
	float4 p : SV_POSITION;
	float4 c : COLOR0;
	float2 t : TEXCOORD;
};

[maxvertexcount(36)]
void GS(in point VS_OUT vIn[1], inout TriangleStream<G_OUT> vOut)
{
	float3 right = g_vXAxis.xyz * g_vSize.x;
	float3 up = g_vYAxis.xyz * g_vSize.y;
	float3 back = g_vZAxis.xyz * g_vSize.z;
	// Strip으로 구성한다.
	float3 g_pos[8] =
	{
		vIn[0].p.xyz - right - up - back,
		vIn[0].p.xyz - right - up + back,
		vIn[0].p.xyz - right + up - back,
		vIn[0].p.xyz - right + up + back,
		vIn[0].p.xyz + right - up - back,
		vIn[0].p.xyz + right - up + back,
		vIn[0].p.xyz + right + up - back,
		vIn[0].p.xyz + right + up + back
	};

	int indices[36] = 
	{
		0, 1, 2, // LEFT
		1, 3, 2,
		4, 6, 5, // RIGHT
		5, 6, 7, 
		0, 4, 1, // BOTTOM
		1, 4, 5, 
		2, 3, 6, // TOP
		3, 7, 6, 
		0, 2, 4, // BACK
		2, 6, 4, 
		1, 5, 3, // FRONT
		3, 5, 7
	};

	G_OUT vVertex;
	for (int i = 0; i < 36; i += 3)
	{
		float4 v[3] = 
		{
			float4(g_pos[indices[i + 0]], 1),
			float4(g_pos[indices[i + 1]], 1),
			float4(g_pos[indices[i + 2]], 1)
		};
	
		for(int j = 0; j < 3; j++)
		{
			vVertex.p = mul(v[j], g_matComb);
			vVertex.c = vIn[0].c;
			vVertex.t = vIn[0].t;
			vOut.Append(vVertex);
		}
		vOut.RestartStrip();
	}
	//G_OUT vVertex;
	//vVertex.c = vIn[0].c;
	//vVertex.t = vIn[0].t;
	//vVertex.p = mul(float4(g_pos[1], 1), g_matComb);
	//vOut.Append(vVertex);
	//vVertex.p = mul(float4(g_pos[5], 1), g_matComb);
	//vOut.Append(vVertex);
	//vVertex.p = mul(float4(g_pos[3], 1), g_matComb);
	//vOut.Append(vVertex);
}

struct PixelOutput
{
	float4 Position : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 Color : SV_TARGET2;
};

float4 PS(VS_OUT input) : SV_TARGET
{
	/*
	PixelOutput output = (PixelOutput)0;

	output.Position = input.p;
	output.Normal = float4(0, 0, 0, 0);
	output.Color = input.c;
	if(output.Color.w < 0.1)
		discard;
	*/
	return input.c;
}
