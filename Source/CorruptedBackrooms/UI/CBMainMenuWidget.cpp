#include "UI/CBMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/CBPlayerController.h"
#include "Misc/Paths.h"

void UCBMainMenuWidget::SetOwnerController(ACBPlayerController* InController)
{
	OwnerController = InController;
}

TSharedRef<SWidget> UCBMainMenuWidget::RebuildWidget()
{
	if (!bLayoutBuilt)
	{
		BuildLayout();
		bLayoutBuilt = true;
	}
	return Super::RebuildWidget();
}

void UCBMainMenuWidget::BuildLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dim"));
	Dim->SetBrushColor(FLinearColor(0.02f, 0.04f, 0.08f, 0.42f));
	UCanvasPanelSlot* DimSlot = Root->AddChildToCanvas(Dim);
	DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	DimSlot->SetOffsets(FMargin());

	auto MakeTitle = [this](const FString& Text, int32 Size, FLinearColor Color) -> UTextBlock*
	{
		UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(FSlateColor(Color));
		Block->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), Size);
		Block->SetFont(Font);
		return Block;
	};

	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));
	MainPanel = MainBox;

	UTextBlock* Title = MakeTitle(TEXT("CORRUPTED BACKROOMS"), 42, FLinearColor(0.95f, 0.97f, 1.f));
	UTextBlock* Subtitle = MakeTitle(TEXT("Небесный перон"), 18, FLinearColor(0.75f, 0.82f, 0.92f, 0.9f));

	if (UVerticalBoxSlot* TitleSlot = MainBox->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
	}
	if (UVerticalBoxSlot* SubSlot = MainBox->AddChildToVerticalBox(Subtitle))
	{
		SubSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 36.f));
		SubSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UButton* PlayButton = MakeMenuButton(TEXT("PlayButton"), TEXT("Играть"), FLinearColor(0.12f, 0.42f, 0.78f));
	UButton* SettingsButton = MakeMenuButton(TEXT("SettingsButton"), TEXT("Настройки"), FLinearColor(0.16f, 0.18f, 0.22f));
	PlayButton->OnClicked.AddDynamic(this, &UCBMainMenuWidget::OnPlayClicked);
	SettingsButton->OnClicked.AddDynamic(this, &UCBMainMenuWidget::OnSettingsClicked);

	if (UVerticalBoxSlot* PlaySlot = MainBox->AddChildToVerticalBox(PlayButton))
	{
		PlaySlot->SetPadding(FMargin(0.f, 8.f));
		PlaySlot->SetHorizontalAlignment(HAlign_Center);
	}
	if (UVerticalBoxSlot* SettingsSlot = MainBox->AddChildToVerticalBox(SettingsButton))
	{
		SettingsSlot->SetPadding(FMargin(0.f, 8.f));
		SettingsSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UCanvasPanelSlot* MainSlot = Root->AddChildToCanvas(MainBox);
	MainSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	MainSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	MainSlot->SetAutoSize(true);

	UVerticalBox* SettingsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsBox"));
	SettingsPanel = SettingsBox;
	SettingsBox->SetVisibility(ESlateVisibility::Collapsed);

	UTextBlock* SettingsTitle = MakeTitle(TEXT("Настройки"), 34, FLinearColor::White);
	if (UVerticalBoxSlot* StTitleSlot = SettingsBox->AddChildToVerticalBox(SettingsTitle))
	{
		StTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));
		StTitleSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UTextBlock* SensLabel = MakeTitle(TEXT("Чувствительность мыши"), 18, FLinearColor(0.85f, 0.88f, 0.92f));
	if (UVerticalBoxSlot* SensLabelSlot = SettingsBox->AddChildToVerticalBox(SensLabel))
	{
		SensLabelSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		SensLabelSlot->SetHorizontalAlignment(HAlign_Center);
	}

	USlider* SensSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("SensSlider"));
	SensSlider->SetMinValue(0.2f);
	SensSlider->SetMaxValue(2.5f);
	SensSlider->SetValue(1.f);
	SensSlider->SetLocked(false);
	SensSlider->OnValueChanged.AddDynamic(this, &UCBMainMenuWidget::OnSensitivityChanged);
	if (UVerticalBoxSlot* SliderSlot = SettingsBox->AddChildToVerticalBox(SensSlider))
	{
		SliderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		SliderSlot->SetHorizontalAlignment(HAlign_Fill);
		SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	SensitivityValueText = MakeTitle(TEXT("1.00"), 16, FLinearColor(0.7f, 0.78f, 0.9f));
	if (UVerticalBoxSlot* ValSlot = SettingsBox->AddChildToVerticalBox(SensitivityValueText))
	{
		ValSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 28.f));
		ValSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UButton* BackButton = MakeMenuButton(TEXT("BackButton"), TEXT("Назад"), FLinearColor(0.16f, 0.18f, 0.22f));
	BackButton->OnClicked.AddDynamic(this, &UCBMainMenuWidget::OnBackClicked);
	if (UVerticalBoxSlot* BackSlot = SettingsBox->AddChildToVerticalBox(BackButton))
	{
		BackSlot->SetPadding(FMargin(0.f, 8.f));
		BackSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UCanvasPanelSlot* SettingsSlotCanvas = Root->AddChildToCanvas(SettingsBox);
	SettingsSlotCanvas->SetAnchors(FAnchors(0.5f, 0.5f));
	SettingsSlotCanvas->SetAlignment(FVector2D(0.5f, 0.5f));
	SettingsSlotCanvas->SetAutoSize(true);
	SettingsSlotCanvas->SetSize(FVector2D(420.f, 0.f));
}

UButton* UCBMainMenuWidget::MakeMenuButton(const FName& Name, const FString& Label, const FLinearColor& Color)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	FButtonStyle Style = Button->WidgetStyle;
	Style.Normal.TintColor = FSlateColor(Color);
	Style.Hovered.TintColor = FSlateColor(Color * 1.25f);
	Style.Pressed.TintColor = FSlateColor(Color * 0.8f);
	Style.Normal.DrawAs = ESlateBrushDrawType::Box;
	Style.Hovered.DrawAs = ESlateBrushDrawType::Box;
	Style.Pressed.DrawAs = ESlateBrushDrawType::Box;
	Style.NormalPadding = FMargin(28.f, 14.f);
	Style.PressedPadding = FMargin(28.f, 14.f);
	Button->SetStyle(Style);

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(FText::FromString(Label));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Text->SetJustification(ETextJustify::Center);
	FSlateFontInfo Font(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 22);
	Text->SetFont(Font);
	Button->AddChild(Text);
	return Button;
}

void UCBMainMenuWidget::OnPlayClicked()
{
	if (ACBPlayerController* PC = OwnerController.Get())
	{
		PC->StartGameFromMenu();
	}
}

void UCBMainMenuWidget::OnSettingsClicked()
{
	if (MainPanel)
	{
		MainPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SettingsPanel)
	{
		SettingsPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCBMainMenuWidget::OnBackClicked()
{
	if (SettingsPanel)
	{
		SettingsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MainPanel)
	{
		MainPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCBMainMenuWidget::OnSensitivityChanged(float Value)
{
	if (SensitivityValueText)
	{
		SensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Value)));
	}
	if (ACBPlayerController* PC = OwnerController.Get())
	{
		PC->SetMouseSensitivity(Value);
	}
}
