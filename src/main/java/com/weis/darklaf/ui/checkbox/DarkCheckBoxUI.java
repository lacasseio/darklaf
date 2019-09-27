package com.weis.darklaf.ui.checkbox;

import com.weis.darklaf.icons.EmptyIcon;
import com.weis.darklaf.util.DarkUIUtil;
import com.weis.darklaf.util.GraphicsContext;
import com.weis.darklaf.util.GraphicsUtil;
import org.jetbrains.annotations.Contract;
import org.jetbrains.annotations.NotNull;
import sun.swing.SwingUtilities2;

import javax.swing.*;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.IconUIResource;
import javax.swing.plaf.basic.BasicHTML;
import javax.swing.plaf.metal.MetalCheckBoxUI;
import javax.swing.text.View;
import java.awt.*;
import java.awt.geom.Path2D;

public class DarkCheckBoxUI extends MetalCheckBoxUI {

    private static final int ICON_OFF = 4;
    private static final int ARC_SIZE = 3;
    private static final int SIZE = 13;

    private static Dimension size = new Dimension();
    private static final Rectangle viewRect = new Rectangle();
    private static final Rectangle iconRect = new Rectangle();
    private static final Rectangle textRect = new Rectangle();

    @NotNull
    @Contract("_ -> new")
    public static ComponentUI createUI(final JComponent c) {
        return new DarkCheckBoxUI();
    }

    @Override
    public synchronized void paint(final Graphics g2d, @NotNull final JComponent c) {
        Graphics2D g = (Graphics2D) g2d;
        JCheckBox b = (JCheckBox) c;
        FontMetrics fm = SwingUtilities2.getFontMetrics(c, g, c.getFont());

        Insets i = c.getInsets();
        size = b.getSize(size);
        viewRect.x = i.left;
        viewRect.y = i.top;
        viewRect.width = size.width - (i.right + viewRect.x);
        viewRect.height = size.height - (i.bottom + viewRect.y);
        iconRect.x = iconRect.y = iconRect.width = iconRect.height = 0;
        textRect.x = textRect.y = textRect.width = textRect.height = 0;


        String text = SwingUtilities.layoutCompoundLabel(c, fm, b.getText(), getDefaultIcon(),
                                                         b.getVerticalAlignment(), b.getHorizontalAlignment(),
                                                         b.getVerticalTextPosition(), b.getHorizontalTextPosition(),
                                                         viewRect, iconRect, textRect, b.getIconTextGap());
        paintBackground(c, g);

        Icon icon = getIconBullet(c, g, b);
        if (icon != null) {
            icon.paintIcon(c, g, iconRect.x, iconRect.y);
        } else {
            paintDarkCheck(c,g,b);
        }

        if (text != null) {
            paintText(g, b, textRect, text, fm, getDisabledTextColor());
        }
    }

    protected void paintDarkCheck(final JComponent c, final Graphics2D g, @NotNull final JCheckBox b) {
        GraphicsContext config = new GraphicsContext(g);
        boolean enabled = b.isEnabled();
        g.translate(iconRect.x + ICON_OFF, iconRect.y + ICON_OFF);

        paintCheckBorder(g, enabled, b.hasFocus());
        if (b.isSelected()) {
            paintCheckArrow(g, enabled);
        }
        g.translate(-iconRect.x - ICON_OFF, -iconRect.y - ICON_OFF);
        config.restore();
    }

    public static Icon getIconBullet(final JComponent c, final Graphics2D g, @NotNull final AbstractButton b) {
        var model = b.getModel();
        var icon = b.getIcon();
        if (!model.isEnabled()) {
            if (model.isSelected()) {
                icon = b.getDisabledSelectedIcon();
            } else {
                icon = b.getDisabledIcon();
            }
        } else if (model.isPressed() && model.isArmed()) {
            icon = b.getPressedIcon();
            if (icon == null) {
                // Use selected icon
                icon = b.getSelectedIcon();
            }
        } else if (model.isSelected()) {
            if (b.isRolloverEnabled() && model.isRollover()) {
                icon = b.getRolloverSelectedIcon();
                if (icon == null) {
                    icon = b.getSelectedIcon();
                }
            } else {
                icon = b.getSelectedIcon();
            }
        } else if (b.isRolloverEnabled() && model.isRollover()) {
            icon = b.getRolloverIcon();
        }

        if (icon == null) {
            icon = b.getIcon();
        }
       return icon;
    }



    public static void paintText(@NotNull final Graphics2D g, @NotNull final AbstractButton b,
                           final Rectangle textRect, final String text, final FontMetrics fm,
                          final Color disabledTextColor) {
        GraphicsContext context = GraphicsUtil.setupAntialiasing(g, true, false);
        g.setFont(b.getFont());
        View view = (View) b.getClientProperty(BasicHTML.propertyKey);
        if (view != null) {
            view.paint(g, textRect);
        } else {
            g.setColor(b.isEnabled() ? b.getForeground() : disabledTextColor);
            SwingUtilities2.drawStringUnderlineCharAt(b, g, text,
                                                      b.getDisplayedMnemonicIndex(),
                                                      textRect.x,
                                                      textRect.y + fm.getAscent());
        }
        context.restore();
    }

    static void paintCheckArrow(@NotNull final Graphics2D g, final boolean enabled) {
        var config = GraphicsUtil.setupStrokePainting(g);
        g.setStroke(new BasicStroke(2.0f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
        Color color = enabled ? UIManager.getColor("CheckBox.darcula.selectionEnabledColor")
                              : UIManager.getColor("CheckBox.darcula.selectionDisabledColor");

        g.setPaint(color);
        Path2D check = new Path2D.Float(Path2D.WIND_EVEN_ODD);
        check.moveTo(2.5, 8);
        check.lineTo(5.5, SIZE - 3);
        check.lineTo(SIZE - 2.7, 3);
        g.draw(check);
        config.restore();
    }

    static void paintCheckBorder(@NotNull final Graphics2D g, final boolean enabled, final boolean focus) {
        var g2 = (Graphics2D)g.create();
        Color bgColor = enabled ? UIManager.getColor("CheckBox.darcula.activeFillColor")
                                : UIManager.getColor("CheckBox.darcula.inactiveFillColor");
        Color borderColor = enabled ? UIManager.getColor("CheckBox.darcula.activeBorderColor")
                                    : UIManager.getColor("CheckBox.darcula.inactiveBorderColor");
        g.setColor(bgColor);
        g.fillRoundRect(0, 0, SIZE, SIZE, ARC_SIZE, ARC_SIZE);

        g.setColor(borderColor);
        DarkUIUtil.paintLineBorder(g, 0, 0, SIZE, SIZE, ARC_SIZE, true);

        if (focus) {
            g2.translate(-2,-2);
            g2.setComposite(DarkUIUtil.ALPHA_COMPOSITE);
            DarkUIUtil.paintFocusBorder(g2, SIZE + 4, SIZE + 4, ARC_SIZE, true);
        }
        g2.dispose();
    }

    private void paintBackground(@NotNull final JComponent c, final Graphics2D g) {
        if (c.isOpaque()) {
            g.setColor(c.getBackground());
            g.fillRect(0, 0, c.getWidth(), c.getHeight());
        }
    }

    @Override
    public Icon getDefaultIcon() {
        return new IconUIResource(EmptyIcon.create(20));
    }
}
