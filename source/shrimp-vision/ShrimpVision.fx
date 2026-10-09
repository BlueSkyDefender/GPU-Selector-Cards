////---------------//
///**Shrimp Vision**///
//---------------////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//* Shrimp Vision
//* For ReShade 5.0+
//* --------------------------
//* A little shrimp floats over your game. Motion in the picture pushes it around.
//* The prize inside the Locked Card Example (GPU Selector Cards). Keep it shrimple.
//*
//* How it works:
//* 1. The picture is shrunk to 64 x 36 brightness values (fast).
//* 2. Simple motion vectors from that: the change since the last frame and the DDX / DDY gradients.
//* 3. The shrimp's place and speed live in a 1 x 1 texture. Motion around it pushes it.
//* 4. The shrimp is drawn with circles and lines. No image files.
//*
//* This work is licensed under the BSD Zero Clause License, like the GPU Selector example cards.
//* Use it, change it and share it. Given "as is", with no warranty.
//*
//* Have fun,
//* Jose Negrete AKA BlueSkyDefender
//*
//* https://github.com/BlueSkyDefender/GPU-Selector-Cards
//* ---------------------------------
//*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform float Shrimp_Size <
	ui_type = "drag";
	ui_min = 0.05; ui_max = 0.25;
	ui_label = "Shrimp Size";
	ui_tooltip = "How big the shrimp is.\n"
				 "Default is 0.1.";
> = 0.1;

uniform float Push <
	ui_type = "drag";
	ui_min = 0.0; ui_max = 4.0;
	ui_label = "Push Strength";
	ui_tooltip = "How hard motion in the game pushes the shrimp.\n"
				 "Default is 1.0.";
> = 1.0;

uniform float Tint <
	ui_type = "drag";
	ui_min = 0.0; ui_max = 0.5;
	ui_label = "Shrimp Vision Tint";
	ui_tooltip = "A pink tint over the picture.\n"
				 "Default is 0.1, 0 is off.";
> = 0.1;

uniform float Timer < source = "timer"; >;
uniform float FrameTime < source = "frametime"; >;

/////////////////////////////////////////////Shrimp Starts Here/////////////////////////////////////////////////////////////////
#define Flow_W 64
#define Flow_H 36
#define Aspect (BUFFER_WIDTH * BUFFER_RCP_HEIGHT)

texture BackBufferTex : COLOR;

sampler BackBuffer
	{
		Texture = BackBufferTex;
	};

texture LumaT  { Width = Flow_W; Height = Flow_H; Format = R16F;};

sampler Luma
	{
		Texture = LumaT;
	};

texture PastLumaT  { Width = Flow_W; Height = Flow_H; Format = R16F;};

sampler PastLuma
	{
		Texture = PastLumaT;
	};

texture FlowT  { Width = Flow_W; Height = Flow_H; Format = RG16F;};

sampler Flow
	{
		Texture = FlowT;
	};

texture ShrimpT  { Width = 1; Height = 1; Format = RGBA32F;};

sampler Shrimp
	{
		Texture = ShrimpT;
		MagFilter = POINT;
		MinFilter = POINT;
		MipFilter = POINT;
	};

texture PastShrimpT  { Width = 1; Height = 1; Format = RGBA32F;};

sampler PastShrimp
	{
		Texture = PastShrimpT;
		MagFilter = POINT;
		MinFilter = POINT;
		MipFilter = POINT;
	};

///////////////////////////////////////////////////////////Motion/////////////////////////////////////////////////////////////////
float Luma_Out(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	return dot(tex2D(BackBuffer, texcoord).rgb, float3(0.299, 0.587, 0.114));
}

float2 Flow_Out(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float L = tex2D(Luma, texcoord).x;
	float Change = L - tex2D(PastLuma, texcoord).x;
	float2 Gradient = float2(ddx(L), ddy(L));
	float Edge = dot(Gradient, Gradient);

	//Flat places and tiny changes tell us nothing about motion
	if (Edge < 0.0001 || abs(Change) < 0.01)
		return 0;

	return clamp(-Change * Gradient / (Edge + 0.001), -2.0, 2.0);
}

///////////////////////////////////////////////////////////Shrimp/////////////////////////////////////////////////////////////////
float4 Shrimp_Move(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float4 S = tex2Dlod(PastShrimp, float4(0.5, 0.5, 0, 0)); //xy = place, zw = speed

	//First frame: start in the middle
	if (S.x <= 0 && S.y <= 0)
		S = float4(0.5, 0.5, 0.1, 0.05);

	float DT = clamp(FrameTime * 0.001, 0.0, 0.1);

	//The motion under the shrimp pushes it
	float2 P = tex2Dlod(Flow, float4(S.xy, 0, 0)).xy / float2(Flow_W, Flow_H);

	//It also swims a little by itself
	float T = Timer * 0.001;
	float2 Swim = float2(sin(T * 0.37), cos(T * 0.29)) * 0.05;

	float2 Speed = S.zw + (P * Push * 3.0 / max(DT, 0.001) + Swim) * DT * 3.0;
	Speed *= exp(-1.5 * DT); //Water slows it down
	Speed = clamp(Speed, -1.0, 1.0);

	float2 Place = S.xy + Speed * DT;

	//Bounce off the edges of the screen
	float2 Margin = float2(Shrimp_Size * 0.6 / Aspect, Shrimp_Size * 0.6);
	if (Place.x < Margin.x || Place.x > 1 - Margin.x)
		Speed.x = -Speed.x;
	if (Place.y < Margin.y || Place.y > 1 - Margin.y)
		Speed.y = -Speed.y;

	return float4(clamp(Place, Margin, 1 - Margin), Speed);
}

float4 Shrimp_Keep(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	return tex2Dlod(Shrimp, float4(0.5, 0.5, 0, 0));
}

float Luma_Keep(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	return tex2D(Luma, texcoord).x;
}

///////////////////////////////////////////////////////////Drawing/////////////////////////////////////////////////////////////////
float Circle(float2 p, float2 c, float r)
{
	return length(p - c) - r;
}

float Line(float2 p, float2 a, float2 b, float w)
{
	float2 pa = p - a, ba = b - a;
	float h = saturate(dot(pa, ba) / dot(ba, ba));
	return length(pa - ba * h) - w;
}

float3 Shrimp_Out(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float3 C = tex2D(BackBuffer, texcoord).rgb;
	C = lerp(C, C * float3(1.1, 0.9, 0.95) + float3(0.05, 0, 0.02), Tint);

	float4 S = tex2Dlod(Shrimp, float4(0.5, 0.5, 0, 0));
	float2 p = (texcoord - S.xy) * float2(Aspect, 1) / Shrimp_Size;

	//Far from the shrimp: done
	if (dot(p, p) > 2.0)
		return C;

	//Face the way it swims
	float2 H = length(S.zw) > 0.01 ? normalize(S.zw) : float2(1, 0);
	p = float2(dot(p, H), dot(p, float2(-H.y, H.x)));
	if (H.x >= 0)
		p.y = -p.y;

	//Body: shell pieces on a curled line, big at the head and small at the tail
	float Body = 1000, Stripe = 1000, W = sin(Timer * 0.004) * 0.2;
	[unroll]
	for (int i = 0; i < 7; i++)
	{
		float k = i / 6.0;
		float a = lerp(0.1, 3.4, k) + W * k;
		float d = Circle(p, float2(cos(a), sin(a)) * 0.4, lerp(0.22, 0.08, k));
		Body = min(Body, d);
		Stripe = min(Stripe, abs(d + 0.05));
	}

	//Tail fan, legs and antennae
	float a = 3.4 + W;
	float2 Tail = float2(cos(a), sin(a)) * 0.4;
	Body = min(Body, Circle(p, Tail + float2(0.05, -0.12), 0.09));
	float Thin = Line(p, float2(0.5, 0.1), float2(1.0, 0.5 + W * 0.3), 0.015);
	Thin = min(Thin, Line(p, float2(0.5, 0.05), float2(1.05, 0.3 - W * 0.3), 0.015));
	Thin = min(Thin, Line(p, float2(0.2, -0.15), float2(0.25, -0.45), 0.02));
	Thin = min(Thin, Line(p, float2(0.0, -0.2), float2(0.0, -0.5), 0.02));
	float Eye = Circle(p, float2(0.45, 0.12), 0.05);

	//Colors
	float3 Shell = lerp(float3(1.0, 0.45, 0.35), float3(1.0, 0.72, 0.6), saturate(-p.y + 0.3));
	float3 Dark = float3(0.6, 0.15, 0.12);
	Shell = lerp(Dark, Shell, smoothstep(0.0, 0.02, Stripe - 0.005));
	Shell = lerp(Dark, Shell, smoothstep(-0.03, -0.01, Body));
	Shell = lerp(float3(0.05, 0.03, 0.05), Shell, smoothstep(0.0, 0.015, Eye));

	C = lerp(C, Dark, 1 - smoothstep(0.0, 0.015, Thin));
	return lerp(C, Shell, 1 - smoothstep(0.0, 0.02, Body));
}

///////////////////////////////////////////////////////////ReShade.fxh/////////////////////////////////////////////////////////////
// Vertex shader generating a triangle covering the entire screen
void PostProcessVS(in uint id : SV_VertexID, out float4 position : SV_Position, out float2 texcoord : TEXCOORD)
{
	texcoord.x = (id == 2) ? 2.0 : 0.0;
	texcoord.y = (id == 1) ? 2.0 : 0.0;
	position = float4(texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}

technique Shrimp_Vision
< ui_tooltip = "A little shrimp floats over your game.\n"
			   "Motion in the picture pushes it around.\n"
			   "Keep it shrimple."; >
	{
			pass Luma
		{
			VertexShader = PostProcessVS;
			PixelShader = Luma_Out;
			RenderTarget = LumaT;
		}
			pass Flow
		{
			VertexShader = PostProcessVS;
			PixelShader = Flow_Out;
			RenderTarget = FlowT;
		}
			pass Move
		{
			VertexShader = PostProcessVS;
			PixelShader = Shrimp_Move;
			RenderTarget = ShrimpT;
		}
			pass KeepShrimp
		{
			VertexShader = PostProcessVS;
			PixelShader = Shrimp_Keep;
			RenderTarget = PastShrimpT;
		}
			pass KeepLuma
		{
			VertexShader = PostProcessVS;
			PixelShader = Luma_Keep;
			RenderTarget = PastLumaT;
		}
			pass Draw
		{
			VertexShader = PostProcessVS;
			PixelShader = Shrimp_Out;
		}
	}
