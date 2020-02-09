#ifndef HILDONIMKEYBOARDASSISTANTSCV_H
#define HILDONIMKEYBOARDASSISTANTSCV_H

#define HILDON_IM_TYPE_KEYBOARD_ASSISTANT_SCV \
        (hildon_im_keyboard_assistant_scv_get_type())

#define HILDON_IM_KEYBOARD_ASSISTANT_SCV(obj) \
        GTK_CHECK_CAST(obj, hildon_im_keyboard_assistant_scv_get_type(), \
                       HildonIMKeyboardAssistantSCV)
#define HILDON_IM_KEYBOARD_ASSISTANT_SCV_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_keyboard_assistant_scv_get_type, \
                             HildonIMKeyboardAssistantSCVClass)
#define HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(obj) \
        GTK_CHECK_TYPE(obj, HILDON_IM_TYPE_KEYBOARD_ASSISTANT_SCV)

typedef struct _HildonIMKeyboardAssistantSCV HildonIMKeyboardAssistantSCV;

GType hildon_im_keyboard_assistant_scv_get_type();
HildonIMKeyboardAssistantSCV *hildon_im_keyboard_assistant_scv_new(HildonIMUI *ui);

#endif // HILDONIMKEYBOARDASSISTANTSCV_H
