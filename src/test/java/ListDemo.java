import com.weis.darklaf.LafManager;
import com.weis.darklaf.theme.Theme;

import javax.swing.*;

public final class ListDemo {

    public static void main(final String[] args) {
        SwingUtilities.invokeLater(() -> {
            LafManager.install();
            var f = new JFrame("frame");
            JPanel p = new JPanel();

            String[] week = {"Monday", "Tuesday", "Wednesday",
                    "Thursday", "Friday", "Saturday", "Sunday"};
            var list = new JList<>(week);

            list.setSelectedIndex(2);
            p.add(list);
            f.add(p);
            f.setSize(400, 400);
            f.setVisible(true);
        });
    }
}