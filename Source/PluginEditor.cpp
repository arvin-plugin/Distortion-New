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
    // Set the background to 200x359 size
    Rectangle<float> bounds(0, 0, 200, 359);
    g.drawImage(backgroundImage, bounds);
}

//==============================================================================

KnobLookAndFeel::KnobLookAndFeel(Image knobImage) : knobImage(knobImage) {}

void KnobLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        Slider& slider)
{
    const int numFrames = knobImage.getHeight() / knobImage.getWidth(); // Calculate number of frames
    const int frameIndex = static_cast<int>(sliderPos * (numFrames - 1));  // Frame index based on slider position
    const int frameHeight = knobImage.getWidth();  // Frame height is 64 (same as width, since sprite is 64x1984)

    // Calculate the size and position to center the knob in the available area
    const int knobWidth = 64;   // Width of the knob
    const int knobHeight = 64;  // Height of the knob
    const int centerX = x + (width - knobWidth) / 2;   // Center horizontally
    const int centerY = y + (height - knobHeight) / 2; // Center vertically

    // Draw the appropriate frame from the knob sprite
    g.drawImage(knobImage, centerX, centerY, knobWidth, knobHeight, 
                0, frameIndex * frameHeight,  // Source rectangle (from sprite sheet)
                knobWidth, frameHeight);      // Frame size (64x64)
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

            Label* aLabel;
            labels.add (aLabel = new Label (parameter->name, parameter->name));
            aLabel->attachToComponent (components.getLast(), true);
            addAndMakeVisible (aLabel);

            components.getLast()->setName (parameter->name);
            components.getLast()->setComponentID (parameter->paramID);
            addAndMakeVisible (components.getLast());
        }
    }

    editorHeight += components.size() * editorPadding;
    setSize (editorWidth, editorHeight);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor() {}

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
