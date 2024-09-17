#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================

BackgroundComponent::BackgroundComponent()
{
    // Load the background image from binary resources
    backgroundImage = ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
}

void BackgroundComponent::paint(Graphics& g)
{
    g.drawImage(backgroundImage, getLocalBounds().toFloat());
}

//==============================================================================

KnobLookAndFeel::KnobLookAndFeel(Image knobImage) : knobImage(knobImage) {}

void KnobLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        Slider& slider)
{
    g.drawImage(knobImage, x, y, width, height, 0, 0, knobImage.getWidth(), knobImage.getHeight());
    // Draw the rotary part
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float radius = jmin(width * 0.5f, height * 0.5f) - 10.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    Path p;
    p.addLineSegment(Line<float>(centreX, centreY, centreX + radius * std::cos(angle), centreY - radius * std::sin(angle)), 2.0f);
    g.setColour(Colours::white);
    g.strokePath(p, PathStrokeType(2.0f));
}

//==============================================================================

DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int comboBoxCounter = 0;

    int editorHeight = 2 * editorMargin;
    
    // Add background component
    addAndMakeVisible(backgroundComponent = new BackgroundComponent());

    for (int i = 0; i < parameters.size(); ++i) {
        if (const AudioProcessorParameterWithID* parameter =
                dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {

            if (processor.parameters.parameterTypes[i] == "Slider") {
                // Create a custom slider
                Slider* aSlider;
                sliders.add (aSlider = new Slider());
                aSlider->setSliderStyle (Slider::Rotary);
                aSlider->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
                aSlider->setTextValueSuffix (parameter->label);

                // Load knob images from binary resources
                Image knobImage = ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
                aSlider->setLookAndFeel(new KnobLookAndFeel(knobImage));

                SliderAttachment* aSliderAttachment;
                sliderAttachments.add (aSliderAttachment =
                    new SliderAttachment (processor.parameters.apvts, parameter->paramID, *aSlider));

                components.add (aSlider);
                editorHeight += sliderHeight;
            }

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ToggleButton") {
                ToggleButton* aButton;
                toggles.add (aButton = new ToggleButton());
                aButton->setToggleState (parameter->getDefaultValue(), dontSendNotification);

                ButtonAttachment* aButtonAttachment;
                buttonAttachments.add (aButtonAttachment =
                    new ButtonAttachment (processor.parameters.apvts, parameter->paramID, *aButton));

                components.add (aButton);
                editorHeight += buttonHeight;
            }

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ComboBox") {
                ComboBox* aComboBox;
                comboBoxes.add (aComboBox = new ComboBox());
                aComboBox->setEditableText (false);
                aComboBox->setJustificationType (Justification::left);
                aComboBox->addItemList (processor.parameters.comboBoxItemLists[comboBoxCounter++], 1);

                ComboBoxAttachment* aComboBoxAttachment;
                comboBoxAttachments.add (aComboBoxAttachment =
                    new ComboBoxAttachment (processor.parameters.apvts, parameter->paramID, *aComboBox));

                components.add (aComboBox);
                editorHeight += comboBoxHeight;
            }

            //======================================

            Label* aLabel;
            labels.add (aLabel = new Label (parameter->name, parameter->name));
            aLabel->attachToComponent (components.getLast(), true);
            addAndMakeVisible (aLabel);

            components.getLast()->setName (parameter->name);
            components.getLast()->setComponentID (parameter->paramID);
            addAndMakeVisible (components.getLast());
        }
    }

    //======================================

    editorHeight += components.size() * editorPadding;
    setSize (editorWidth, editorHeight);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

void DistortionAudioProcessorEditor::paint (Graphics& g)
{
    // Background already handled by BackgroundComponent
}

void DistortionAudioProcessorEditor::resized()
{
    Rectangle<int> r = getLocalBounds().reduced (editorMargin);
    r = r.removeFromRight (r.getWidth() - labelWidth);

    backgroundComponent->setBounds (getLocalBounds());

    for (int i = 0; i < components.size(); ++i) {
        if (Slider* aSlider = dynamic_cast<Slider*> (components[i]))
            components[i]->setBounds (r.removeFromTop (sliderHeight));

        if (ToggleButton* aButton = dynamic_cast<ToggleButton*> (components[i]))
            components[i]->setBounds (r.removeFromTop (buttonHeight));

        if (ComboBox* aComboBox = dynamic_cast<ComboBox*> (components[i]))
            components[i]->setBounds (r.removeFromTop (comboBoxHeight));

        r = r.removeFromBottom (r.getHeight() - editorPadding);
    }
}

