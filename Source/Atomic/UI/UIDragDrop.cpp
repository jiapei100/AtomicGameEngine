
#include <ThirdParty/TurboBadger/tb_widgets.h>

#include "../IO/Log.h"
#include "../Input/Input.h"
#include "../Input/InputEvents.h"

#include "UI.h"
#include "UIEvents.h"
#include "UIWidget.h"
#include "UILayout.h"
#include "UIFontDescription.h"
#include "UITextField.h"
#include "UIImageWidget.h"
#include "UISelectList.h"
#include "UIDragDrop.h"
#include "UIDragObject.h"

#ifdef ATOMIC_PLATFORM_OSX
#include "UIDragDropMac.h"
#elif ATOMIC_PLATFORM_WINDOWS
#include "UIDragDropWindows.h"
#endif

using namespace tb;

namespace Atomic
{

UIDragDrop::UIDragDrop(Context* context) : Object(context)
{
    SubscribeToEvent(E_MOUSEMOVE, HANDLER(UIDragDrop,HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONUP, HANDLER(UIDragDrop,HandleMouseUp));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, HANDLER(UIDragDrop,HandleMouseDown));

    SharedPtr<UIFontDescription> fd(new UIFontDescription(context));
    fd->SetId("Vera");
    fd->SetSize(12);

    dragText_ = new UITextField(context);
    dragText_->SetFontDescription(fd);
    dragText_->SetGravity(UI_GRAVITY_TOP);

    dragLayout_ = new UILayout(context);
    dragLayout_->AddChild(dragText_);

    dragLayout_->SetLayoutSize(UI_LAYOUT_SIZE_PREFERRED);
    dragLayout_->SetVisibility(UI_WIDGET_VISIBILITY_GONE);

    // put into hierarchy so we aren't pruned
    TBWidget* root = GetSubsystem<UI>()->GetRootWidget();
    root->AddChild(dragLayout_->GetInternalWidget());

    InitDragAndDrop(this);

}

UIDragDrop::~UIDragDrop()
{

}

void UIDragDrop::DragEnd()
{
    SharedPtr<UIDragObject> dragObject = dragObject_;
    SharedPtr<UIWidget> currentTargetWidget = currentTargetWidget_;

    // clean up
    currentTargetWidget_ = 0;
    dragObject_ = 0;
    dragSelectObject_ = 0;
    dragLayout_->SetVisibility(UI_WIDGET_VISIBILITY_GONE);

    if (currentTargetWidget.Null())
    {
        return;
    }

    VariantMap dropData;
    dropData[DragEnded::P_TARGET] = currentTargetWidget;
    dropData[DragEnded::P_DRAGOBJECT] = dragObject;
    dragObject->SendEvent(E_DRAGENDED, dropData);
}

void UIDragDrop::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    Input* input = GetSubsystem<Input>();

    if (!input->IsMouseVisible())
        return;

    if ((eventData[P_BUTTONS].GetUInt() & MOUSEB_LEFT) && TBWidget::hovered_widget)
    {
        // see if we have a widget
        TBWidget* tbw = TBWidget::hovered_widget;

        while(tbw && !tbw->GetDelegate())
        {
            tbw = tbw->GetParent();
        }

        if (!tbw)
            return;

        UIWidget* widget = (UIWidget*) tbw->GetDelegate();

        currentTargetWidget_ = widget;

        dragSelectObject_ = widget->GetDragObject();

    }

}

void UIDragDrop::HandleMouseUp(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonUp;

    dragSelectObject_ = 0;

    if (dragObject_.Null())
        return;

    if (!(eventData[P_BUTTON].GetInt() ==  MOUSEB_LEFT))
        return;

    DragEnd();

}

void UIDragDrop::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();

    if (!input->IsMouseVisible())
    {
        dragObject_ = 0;
        dragSelectObject_ = 0;
        return;
    }

    if (dragSelectObject_.NotNull())
    {
        dragObject_ = dragSelectObject_;
        dragSelectObject_ = 0;
    }

    if (dragObject_.Null())
        return;

    // initialize if necessary
    if (dragLayout_->GetVisibility() == UI_WIDGET_VISIBILITY_GONE)
    {
        dragLayout_->GetInternalWidget()->SetZ(WIDGET_Z_TOP);
        dragLayout_->SetVisibility(UI_WIDGET_VISIBILITY_VISIBLE);
        dragText_->SetText(dragObject_->GetText());

        UIPreferredSize* sz = dragLayout_->GetPreferredSize();
        dragLayout_->SetRect(IntRect(0, 0, sz->GetMinWidth(), sz->GetMinHeight()));
    }

    using namespace MouseMove;

    int x = eventData[P_X].GetInt();
    int y = eventData[P_Y].GetInt();

    // see if we have a widget
    TBWidget* tbw = TBWidget::hovered_widget;

    while(tbw && !tbw->GetDelegate())
    {
        tbw = tbw->GetParent();
    }

    if (!tbw)
        return;

    UIWidget* hoverWidget = (UIWidget*) tbw->GetDelegate();

    if (hoverWidget != currentTargetWidget_)
    {
        if (currentTargetWidget_)
        {
            VariantMap exitData;
            exitData[DragExitWidget::P_WIDGET] = currentTargetWidget_;
            exitData[DragExitWidget::P_DRAGOBJECT] = dragObject_;
            SendEvent(E_DRAGEXITWIDGET, exitData);
        }

        currentTargetWidget_ = hoverWidget;

        VariantMap enterData;
        enterData[DragEnterWidget::P_WIDGET] = currentTargetWidget_;
        enterData[DragEnterWidget::P_DRAGOBJECT] = dragObject_;
        SendEvent(E_DRAGENTERWIDGET, enterData);

    }

    dragLayout_->SetPosition(x, y - 20);

}

void UIDragDrop::FileDragEntered()
{
    dragObject_ = new UIDragObject(context_);
    //dragObject_->SetText("Files...");
}

void UIDragDrop::FileDragAddFile(const String& filename)
{
    dragObject_->AddFilename(filename);
}

void UIDragDrop::FileDragConclude()
{
    DragEnd();
}


}
