package com.weis.darklaf.components;

import org.jetbrains.annotations.NotNull;

import javax.swing.*;
import java.awt.*;
import java.beans.PropertyVetoException;

/**
 * @author Jannis Weis
 */
public class ClosableTabbedPane extends JTabbedPane {

    @Override
    public void insertTab(final String title, final Icon icon, final Component component,
                          final String tip, final int index) {
        if (notifyVetoableChangeListeners(new TabPropertyChangeEvent(this, "tabOpened",
                                                                     null, component, index))) {
            return;
        }
        super.insertTab(title, icon, component, tip, index);
        setTabComponentAt(indexOfComponent(component), new ClosableTabComponent(this));
        notifyTabListeners(new TabEvent(this, TabEvent.TAB_OPENED, "tabOpened", index));
    }

    @Override
    public void setTabComponentAt(final int index, final Component component) {
        if (component instanceof ClosableTabComponent) {
            ((ClosableTabComponent) component).setTabbedPane(this);
        }
        super.setTabComponentAt(index, component);
    }

    @Override
    public void removeTabAt(final int index) {
        checkIndex(index);
        if (notifyVetoableChangeListeners(new TabPropertyChangeEvent(this, "tabClosed",
                                                                     getComponentAt(index), null, index))) {
            return;
        }
        notifyTabListeners(new TabEvent(this, TabEvent.TAB_CLOSED, "tabClosed", index));
        super.removeTabAt(index);
    }

    private void checkIndex(final int index) {
        int tabCount = getTabCount();
        if (index < 0 || index >= tabCount) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Tab count: " + tabCount);
        }
    }

    private boolean notifyVetoableChangeListeners(final TabPropertyChangeEvent e) {
        try {
            var listeners = getVetoableChangeListeners();
            for (var l : listeners) {
                l.vetoableChange(e);
            }
        } catch (PropertyVetoException ex) {
            return true;
        }
        return false;
    }

    private void notifyTabListeners(@NotNull final TabEvent event) {
        var listeners = listenerList.getListeners(TabListener.class);
        switch (event.getID()) {
            case TabEvent.TAB_CLOSED:
                for (var l : listeners) {
                    l.tabClosed(event);
                }
                break;
            case TabEvent.TAB_OPENED:
                for (var l : listeners) {
                    l.tabOpened(event);
                }
                break;
        }
    }

    public void addTabListener(final TabListener listener) {
        listenerList.add(TabListener.class, listener);
    }

    public void removeTabListener(final TabListener listener) {
        listenerList.remove(TabListener.class, listener);
    }
}
