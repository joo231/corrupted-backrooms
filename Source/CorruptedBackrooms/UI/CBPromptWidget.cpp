#include "UI/CBPromptWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Misc/Paths.h"

TSharedRef<SWidget> UCBPromptWidget::RebuildWidget()
{
	if (!bLayoutBuilt)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Prompt"));
		PromptText->SetText(FText::GetEmpty());
		PromptText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		PromptText->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 22);
		PromptText->SetFont(Font);

		UCanvasPanelSlot* PromptSlot = Root->AddChildToCanvas(PromptText);
		PromptSlot->SetAnchors(FAnchors(0.5f, 1.f));
		PromptSlot->SetAlignment(FVector2D(0.5f, 1.f));
		PromptSlot->SetPosition(FVector2D(0.f, -80.f));
		PromptSlot->SetAutoSize(true);

		bLayoutBuilt = true;
	}
	return Super::RebuildWidget();
}

void UCBPromptWidget::SetPrompt(const FText& InText)
{
	if (PromptText)
	{
		PromptText->SetText(InText);
	}
}

void UCBPromptWidget::SetPromptVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
