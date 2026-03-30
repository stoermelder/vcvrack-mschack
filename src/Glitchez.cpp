#include "mscHack.hpp"

//-----------------------------------------------------
// General Definition
//-----------------------------------------------------
struct Glitchez_AMT_Knob : MSCH_Widget_Knob1 
{
    Glitchez_AMT_Knob()
    {
        set( DWRGB( 255, 255, 255 ), DWRGB( 255, 255, 255 ), 7.5f );
    }
};

struct Glitchez_Knob : MSCH_Widget_Knob1 
{
    Glitchez_Knob()
    {
        set( DWRGB( 0, 255, 255 ), DWRGB( 0, 255, 255 ), 7.5f );
    }
};

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Glitchez : Module
{
	enum ParamIds 
    {
        PARAM_FREQ,
        PARAM_FREQ_LFO_AMT,

        PARAM_THRESH,
        PARAM_THRESH_LFO_AMT,

        nPARAMS
    };

	enum InputIds 
    {
        IN_TRIG,
        IN_AUDIO,
        INPUT_INCV_FREQ,
        INPUT_INCV_THRESH,
        IN_MUTE_GATE,

        nINPUTS 
	};

	enum OutputIds 
    {
		OUT,
        nOUTPUTS
	};

	enum LightIds 
    {
        nLIGHTS
	};

	bool            m_bInitialized = false;

    // Contructor
    Glitchez()
    {
        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        configParam( PARAM_FREQ, 0.0, 1.0, 0.0, "Glitch Length (ms)" );
        configParam( PARAM_FREQ_LFO_AMT, 0.0, 1.0, 0.0, "Glitch Length Mod Amount" );
        configParam( PARAM_THRESH, 0.0, 1.0, 0.0, "Threshold" );
        configParam( PARAM_THRESH_LFO_AMT, 0.0, 1.0, 0.0, "Threshold Mod Amount" );

        configInput( IN_TRIG, "Trigger" );
        configInput( IN_AUDIO, "Audio" );
        configInput( INPUT_INCV_FREQ, "Glitch Length Mod" );
        configInput( INPUT_INCV_THRESH, "Threshold Mod" );
        configInput( IN_MUTE_GATE, "Mute Gate" );
        configOutput( OUT, "Glitched Audio" );
    }

    dsp::SchmittTrigger m_SchmitTrigRand;
    dsp::PulseGenerator m_PulseClock;

    MyLEDButton     		*m_pButtonMute = NULL;
    bool                    m_bMute = false;

    float   ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam );

    // Overrides 
    void    JsonParams( bool bTo, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
};

Glitchez GlitchezBrowser;

//-----------------------------------------------------
// MyLEDButton_Glitchez_Mute
//-----------------------------------------------------
void MyLEDButton_Glitchez_Mute( void *pClass, int id, bool bOn )
{
    Glitchez *mymodule;
    mymodule = (Glitchez*)pClass;
    mymodule->m_bMute = bOn;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Glitchez_Widget : ModuleWidget 
{

    Glitchez_Widget( Glitchez *module )
{
    Glitchez *pmod;

    setModule(module);

    if( !module )
        pmod = &GlitchezBrowser;
    else
        pmod = module;

    //box.size = Vec( 15*5, 380 );
    setPanel(APP->window->loadSvg(asset::plugin( thePlugin, "res/Glitchez.svg")));

    // threshold
    addInput(createInput<MyPortInSmall>( Vec( 3, 46 ), module, Glitchez::IN_TRIG ) );
    addParam( createParam<Glitchez_Knob>( Vec( 35, 46 ), module, Glitchez::PARAM_THRESH ) );
    addInput(createInput<MyPortInSmall>( Vec( 33, 70 ), module, Glitchez::INPUT_INCV_THRESH ) );
    addParam( createParam<Glitchez_AMT_Knob>( Vec( 53, 61 ), module, Glitchez::PARAM_THRESH_LFO_AMT ) );

    // glitch frequency
    addInput(createInput<MyPortInSmall>( Vec( 3, 146 ), module, Glitchez::IN_AUDIO ) );
    addParam( createParam<Glitchez_Knob>( Vec( 35, 146 ), module, Glitchez::PARAM_FREQ ) );
    addInput(createInput<MyPortInSmall>( Vec( 33, 169 ), module, Glitchez::INPUT_INCV_FREQ ) );
    addParam( createParam<Glitchez_AMT_Knob>( Vec( 53, 160 ), module, Glitchez::PARAM_FREQ_LFO_AMT ) );

    // output
    pmod->m_pButtonMute = new MyLEDButton( 28, 256, 12, 12, 14.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, MyLEDButton_Glitchez_Mute );
    addChild( pmod->m_pButtonMute );

    addInput(createInput<MyPortInSmall>( Vec( 3, 252 ), module, Glitchez::IN_MUTE_GATE ) );
    addOutput(createOutput<MyPortOutSmall>( Vec( 50, 252 ), module, Glitchez::OUT ) );

    addChild(createWidget<ScrewSilver>(Vec(30, 0)));
    addChild(createWidget<ScrewSilver>(Vec(30, 365)));

    if( module )
    {
        module->onReset();
        module->m_bInitialized = true;
    }
}
};

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Glitchez::JsonParams( bool bTo, json_t *root)
{
    JsonDataBool    ( bTo, "m_bMute", root, &m_bMute, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Glitchez::dataToJson()
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
void Glitchez::dataFromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    if( !m_bInitialized )
        return;

    m_pButtonMute->Set( m_bMute );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Glitchez::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Glitchez::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   ProcessCV
//
//-----------------------------------------------------
float Glitchez::ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam )
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
void Glitchez::process(const ProcessArgs &args)
{
    float out = 0.0f, fpulse = 0.0f, thr = 0.0f, freq;

	if( !m_bInitialized )
		return;

    if( inputs[ IN_MUTE_GATE ].isConnected() )
    {
        if( inputs[ IN_MUTE_GATE ].getVoltage() >= 0.00001 )
        {
            m_bMute = true;
            m_pButtonMute->Set( true );
        }
        else
        {
            m_bMute = false;
            m_pButtonMute->Set( false );
        }
    }

    out = inputs[ IN_AUDIO ].getNormalVoltage( 0.0f );

    freq = 1.0 + ( 10.0f * ProcessCV( PARAM_FREQ, INPUT_INCV_FREQ, PARAM_FREQ_LFO_AMT, false ) );
    thr = AUDIO_MAX * ProcessCV( PARAM_THRESH, INPUT_INCV_THRESH, PARAM_THRESH_LFO_AMT, false );

	// randomize trigger
	if( m_SchmitTrigRand.process( inputs[ IN_TRIG ].getNormalVoltage( 0.0f ) - thr ) )
	{
        m_PulseClock.trigger( 0.001f * freq );
	}

    fpulse = m_PulseClock.process( 1.0 / APP->engine->getSampleRate() ) ? 1.0f : 0.0f;

    if( !m_bMute )
        outputs[ OUT ].setVoltage( fpulse * out );
    else
        outputs[ OUT ].setVoltage( 0.0f );
}

Model *modelGlitchez = createModel<Glitchez, Glitchez_Widget>( "Glitchez" );
