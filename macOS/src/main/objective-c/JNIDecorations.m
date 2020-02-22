/*
 * MIT License
 *
 * Copyright (c) 2020 Jannis Weis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "com_github_weisj_darklaf_platform_macos_JNIDecorationsWindows.h"
#include "Cocoa/Cocoa.h"

#define OBJC(jl) ((id)jlong_to_ptr(jl))
#define FULL_WINDOW_CONTENT 1 << 14
#define TRANSPARENT_TITLE_BAR 1 << 18

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_macos_JNIDecorationsMacOS_installDecorations(JNIEnv *, jclass, jlong)
{
    NSWindow *nsWindow = OBJC(windowPtr);
    window.styleMask |= FULL_WINDOW_CONTENT
    window.styleMask |= TRANSPARENT_TITLE_BAR;
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_macos_JNIDecorationsMacOS_uninstallDecorations(JNIEnv *, jclass, jlong)
{
    NSWindow *nsWindow = OBJC(windowPtr);
    window.styleMask &= ~FULL_WINDOW_CONTENT;
    window.styleMask &= ~TRANSPARENT_TITLE_BAR;
}
