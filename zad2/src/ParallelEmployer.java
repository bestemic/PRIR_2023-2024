import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ParallelEmployer implements Employer {

    private OrderInterface orderInterface;
    private Location exit;
    private final Object lock = new Object();
    private final List<Location> orderedLocations = new ArrayList<>();
    private final Map<Integer, Location> orders = new HashMap<>();
    private final Map<Integer, List<Direction>> ordersResults = new HashMap<>();

    @Override
    public void setOrderInterface(OrderInterface order) {
        orderInterface = order;
        orderInterface.setResultListener(this::processResult);
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        exit = null;
        orderedLocations.add(startLocation);
        runExploration(startLocation, allowedDirections);
        waitForExit();
        return exit;
    }

    private void processResult(Result result) {
        int orderId = result.orderID();
        if (result.type() == LocationType.EXIT) {
            exit = orders.get(orderId);
            notifyExit();
        }
        synchronized (ordersResults) {
            ordersResults.put(orderId, result.allowedDirections());
        }
        synchronized (orders.get(orderId)) {
            orders.get(orderId).notify();
        }
    }

    private void runExploration(Location startLocation, List<Direction> directions) {
        for (Direction direction : directions) {
            Location location = direction.step(startLocation);
            exploreLocation(location);
        }
    }

    private void exploreLocation(Location location) {
        Thread thread = new Thread(() -> {
            int orderId = checkAndOrder(location);
            if (orderId != -1) {
                waitForOrder(orderId);
                if (exit == null) {
                    List<Direction> directions = ordersResults.get(orderId);
                    runExploration(location, directions);
                }
            }
        });
        thread.start();
    }

    private void waitForOrder(int orderId) {
        synchronized (orders.get(orderId)) {
            try {
                if (!ordersResults.containsKey(orderId)) {
                    orders.get(orderId).wait();
                }
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private int checkAndOrder(Location location) {
        int orderId = -1;
        synchronized (orders) {
            if (!isOrdered(location)) {
                orderedLocations.add(location);
                orderId = orderInterface.order(location);
                orders.put(orderId, location);
            }
        }
        return orderId;
    }

    private void notifyExit() {
        synchronized (lock) {
            lock.notify();
        }
    }

    private void waitForExit() {
        synchronized (lock) {
            try {
                lock.wait();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private boolean isOrdered(Location location) {
        return orderedLocations.contains(location);
    }
}