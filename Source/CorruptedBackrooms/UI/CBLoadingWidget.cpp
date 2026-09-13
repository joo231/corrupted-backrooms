#include "UI/CBLoadingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Misc/Paths.h"

TSharedRef<SWidget> UCBLoadingWidget::RebuildWidget()
{
	if (!bLayoutBuilt)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		UBorder* Fill = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Fill"));
		Fill->SetBrushColor(FLinearColor(0.01f, 0.02f, 0.05f, 1.f));
		UCanvasPanelSlot* FillSlot = Root->AddChildToCanvas(Fill);
		FillSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		FillSlot->SetOffsets(FMargin());

		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LoadingText"));
		Text->SetText(FText::FromString(TEXT("Загрузка...")));
		Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 1.f)));
		Text->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 36);
		Text->SetFont(Font);

		UCanvasPanelSlot* TextSlot = Root->AddChildToCanvas(Text);
		TextSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		TextSlot->SetAutoSize(true);

		bLayoutBuilt = true;
	}
	return Super::RebuildWidget();
}
