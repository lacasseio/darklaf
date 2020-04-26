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
 *
 */
package com.github.weisj.darklaf.uiresource;

import java.awt.*;
import java.awt.geom.AffineTransform;
import java.text.AttributedCharacterIterator;
import java.util.Map;

import javax.swing.plaf.FontUIResource;

public class DarkFontUIResource extends FontUIResource {

    public DarkFontUIResource(final String name, final int style, final int size) {
        super(name, style, size);
    }

    public DarkFontUIResource(final Font font) {
        super(font);
    }

    @Override
    public Font deriveFont(final int style) {
        return new DarkFontUIResource(super.deriveFont(style));
    }

    @Override
    public Font deriveFont(final float size) {
        return new DarkFontUIResource(super.deriveFont(size));
    }

    @Override
    public Font deriveFont(final Map<? extends AttributedCharacterIterator.Attribute, ?> attributes) {
        return new DarkFontUIResource(super.deriveFont(attributes));
    }

    @Override
    public Font deriveFont(final AffineTransform trans) {
        return new DarkFontUIResource(super.deriveFont(trans));
    }

    @Override
    public Font deriveFont(final int style, final float size) {
        return new DarkFontUIResource(super.deriveFont(style, size));
    }

    @Override
    public Font deriveFont(final int style, final AffineTransform trans) {
        return new DarkFontUIResource(super.deriveFont(style, trans));
    }
}
