#include "mscHack.hpp"

#define nCHANNELS 9

struct MTM9_AMT_Knob : MSCH_Widget_Knob1 
{
    MTM9_AMT_Knob()
    {
        set( DWRGB( 255, 255, 255 ), DWRGB( 255, 255, 255 ), 5.5f );
    }
};

struct MTM9_Knob : MSCH_Widget_Knob1 
{
    MTM9_Knob()
    {
        set( DWRGB( 255, 255, 255 ), DWRGB( 255, 255, 255 ), 9.5f );
    }
};

struct MTM9_Knob_Snap : MSCH_Widget_Knob1 
{
    MTM9_Knob_Snap()
    {
        Knob::snap = true;
        set( DWRGB( 180, 180, 180 ), DWRGB( 180, 180, 180 ), 9.5f );
    }
};

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct MTM9 : Module
{
	enum ParamIds 
    {
		PARAM_OCT,
		PARAM_SEMI	        = PARAM_OCT + nCHANNELS,
        PARAM_FINE	        = PARAM_SEMI + nCHANNELS,
        PARAM_FINE_CV_AMT	= PARAM_FINE + nCHANNELS,
        nPARAMS			    = PARAM_FINE_CV_AMT + nCHANNELS
    };

	enum InputIds 
    {
		IN_VOCT,
		IN_GATE	    = IN_VOCT + nCHANNELS,
		IN_FINE_CV	= IN_GATE + nCHANNELS,
        nINPUTS 	= IN_FINE_CV + nCHANNELS
	};

	enum OutputIds 
    {
        OUT_POLY_VOCT,
        OUT_POLY_GATE,
		OUT_VOCT,
        nOUTPUTS	= OUT_VOCT + nCHANNELS,
	};

	enum LightIds 
    {
        ENUMS(PHASE_LIGHT, 3 * nCHANNELS),
        nLIGHTS
	};

    bool            m_bInitialized = false;

    // Contructor
	MTM9()
    {
        char strVal[ 20 ] = {};

        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        for( int i = 0; i < nCHANNELS; i++ )
        {
            snprintf( strVal, sizeof(strVal), "Ch %d Octave", i + 1);
            configParam( PARAM_OCT + i, -5.0f, 5.0f, 0.0f, strVal );
            snprintf( strVal, sizeof(strVal), "Ch %d Semitone", i + 1 );
            configParam( PARAM_SEMI + i, -11.0f, 11.0f, 0.0f, strVal );
            snprintf( strVal, sizeof(strVal), "Ch %d Fine", i + 1 );
            configParam( PARAM_FINE + i, 0.0f, 1.0f, 0.5f, strVal );
            snprintf( strVal, sizeof(strVal), "Ch %d Fine CV Amount", i + 1 );
            configParam( PARAM_FINE_CV_AMT + i, 0.0f, 1.0f, 0.0f, strVal );

            configInput( IN_VOCT + i, string::f("Ch %d V/Oct", i + 1) );
            configInput( IN_GATE + i, string::f("Ch %d Gate", i + 1) );
            configInput( IN_FINE_CV + i, string::f("Ch %d Fine CV", i + 1) );
            configOutput( OUT_VOCT + i, string::f("Ch %d V/Oct", i + 1) );
        }

        configOutput( OUT_POLY_VOCT, "Poly V/Oct" );
        configOutput( OUT_POLY_GATE, "Poly Gate" );
    }

    // Overrides 
    void    JsonParams( bool bTo, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onReset() override;

    float   ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam );
};

// dumb
MTM9 g_MTM9_Browser;

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
struct MTM9_Widget : ModuleWidget 
{

MTM9_Widget( MTM9 *module )
{
	int y;
    MTM9 *pmod;

	//box.size = Vec( 15*12, 380);

    setModule(module);

    if( !module )
        pmod = &g_MTM9_Browser;
    else
        pmod = module;

    setPanel(APP->window->loadSvg(asset::plugin( thePlugin, "res/MTM9.svg")));

	y = 54;

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
		// inputs
		addInput(createInput<MyPortInTiny>( Vec( 7, y ), pmod, MTM9::IN_VOCT + ch ) );
		addInput(createInput<MyPortInTiny>( Vec( 28, y ), pmod, MTM9::IN_GATE + ch ) );
		addInput(createInput<MyPortInTiny>( Vec( 134, y + 6 ), pmod, MTM9::IN_FINE_CV + ch ) );

        // params
        addParam(createParam<MTM9_Knob_Snap>( Vec( 46, y ), pmod, MTM9::PARAM_OCT + ch  ) );
        addParam(createParam<MTM9_Knob_Snap>( Vec( 69, y ), pmod, MTM9::PARAM_SEMI + ch ) );
        addParam(createParam<MTM9_Knob>( Vec( 92, y ), pmod, MTM9::PARAM_FINE + ch  ) );
        addParam(createParam<MTM9_AMT_Knob>( Vec( 118, y + 5 ), pmod, MTM9::PARAM_FINE_CV_AMT + ch ) );

        // outputs
        addOutput(createOutput<MyPortOutTiny>( Vec( 160, y ), pmod, MTM9::OUT_VOCT + ch ) );

        // lights
        addChild(createLight<SmallLight<RedGreenBlueLight>>(Vec(150, y + 1.5f), pmod, MTM9::PHASE_LIGHT + (ch*3)));

        y += 30;
	}

    // poly outputs
    addOutput(createOutput<MyPortOutTiny>( Vec( 160, 322 ), pmod, MTM9::OUT_POLY_VOCT ) );
    addOutput(createOutput<MyPortOutTiny>( Vec( 129, 322 ), pmod, MTM9::OUT_POLY_GATE ) );

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365))); 
	addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

    if( module )
    {
        module->m_bInitialized = true;
        module->onReset();
    }
}
};

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void MTM9::JsonParams( bool bTo, json_t *root)
{
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *MTM9::dataToJson()
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void MTM9::dataFromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    if( !m_bInitialized )
        return;
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void MTM9::onReset()
{
}

//-----------------------------------------------------
// Procedure:   ProcessCV
//
//-----------------------------------------------------
float MTM9::ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam )
{
    float lfo = 0.0f;

    if( inputs[ lfocv ].isConnected() )
        lfo = clamp( inputs[ lfocv ].getVoltage() / CV_MAXn5, -1.0f, 1.0f ) * ( params[ amtparam ].getValue() * params[ amtparam ].getValue() );

    if( bLinearParam )
        return clamp( params[ param ].getValue() + lfo, 0.0f, 1.0f );
    else
        return clamp( ( params[ param ].getValue() * params[ param ].getValue() ) + lfo, 0.0f, 1.0f );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
#define SEMI    ( 1.0f / 12.0f )

void MTM9::process(const ProcessArgs &args)
{
    float voct, gate, fine;
    int count = 0;
    int nPoly = 0;

	if( !m_bInitialized )
		return;

    if(inputs[IN_VOCT].isConnected())
    {
        if( inputs[IN_VOCT].getChannels() )
            nPoly = inputs[IN_VOCT].getChannels();
    }

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
        if(ch < nPoly)
        {
            fine = (ProcessCV(PARAM_FINE + ch, IN_FINE_CV + ch, PARAM_FINE_CV_AMT + ch, true) - 0.5f) * (SEMI * 2);
            voct = inputs[IN_VOCT + ch].getNormalVoltage(0.0f) + params[PARAM_OCT + ch].getValue() + (params[PARAM_SEMI + ch].getValue() * SEMI) + fine;

            outputs[OUT_VOCT + ch].setVoltage(voct);
            outputs[OUT_POLY_VOCT].setVoltage(voct, ch);
            lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(1.0f);
            count++;
        }
        else
        {
            if(inputs[IN_VOCT + ch].isConnected())
            {
                fine = (ProcessCV(PARAM_FINE + ch, IN_FINE_CV + ch, PARAM_FINE_CV_AMT + ch, true) - 0.5f) * (SEMI * 2);
                voct = inputs[IN_VOCT + ch].getNormalVoltage(0.0f) + params[PARAM_OCT + ch].getValue() + (params[PARAM_SEMI + ch].getValue() * SEMI) + fine;

                outputs[OUT_VOCT + ch].setVoltage(voct);
                outputs[OUT_POLY_VOCT].setVoltage(voct, ch);
                lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(1.0f);
                count = ch + 1;
            }
            else
            {
                outputs[OUT_VOCT + ch].setVoltage(0.0f);
                outputs[OUT_POLY_VOCT].setVoltage(0.0f, ch);
                lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(0.f);
            }
        }

        if(inputs[IN_GATE + ch].isConnected())
        {
            gate = inputs[IN_GATE + ch].getNormalVoltage(0.0f);
            outputs[OUT_POLY_GATE].setVoltage(gate, ch);
        }
        else
        {
            outputs[OUT_POLY_GATE].setVoltage(0.0f, ch);
        }
	}

    outputs[ OUT_POLY_VOCT ].setChannels( count );
    outputs[ OUT_POLY_GATE ].setChannels( count );
}

Model *modelMTM9 = createModel<MTM9, MTM9_Widget>( "MTM9" );
