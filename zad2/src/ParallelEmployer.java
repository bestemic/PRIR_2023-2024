import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ParallelEmployer implements Employer {

    private OrderInterface orderInterface;
    private final List<Location> visitedLocations = new ArrayList<>();
    private final List<Location> orderedLocations = new ArrayList<>();
    private final Map<Integer, Location> orders = new HashMap<>();
    private final Map<Integer, List<Direction>> ordersResults = new HashMap<>();


    private Location exit;
    private final Object exitLock = new Object();

    @Override
    public void setOrderInterface(OrderInterface order) {
        orderInterface = order;
        orderInterface.setResultListener(result -> {
            if (result.type().equals(LocationType.EXIT)) {
                exit = orders.get(result.orderID());

                for (int id : orders.keySet()) {
                    synchronized (orders.get(id)) {
                        orders.get(id).notify();
                    }
                }

                synchronized (exitLock) {
                    exitLock.notify();
                }
            } else {
                synchronized (ordersResults) {
                    ordersResults.put(result.orderID(), result.allowedDirections());
                }

                synchronized (orders.get(result.orderID())) {
                    orders.get(result.orderID()).notify();
                }
            }
        });
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        exit = null;
        visitedLocations.add(startLocation);

        for (Direction direction : allowedDirections) {
            Location location = direction.step(startLocation);
            explore(location);
        }

        synchronized (exitLock) {
            try {
                exitLock.wait();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        return exit;
    }

    private void explore(Location location) {
        Thread thread = new Thread(() -> {
            int orderId = -1;
            synchronized (orders) {
                if (!isVisited(location) && !isOrdered(location)) {
                    orderedLocations.add(location);
                    orderId = orderInterface.order(location);
                    orders.put(orderId, location);
                }
            }

            if (orderId != -1) {
                synchronized (orders.get(orderId)) {
                    try {
                        orders.get(orderId).wait();
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                }

                if (exit == null) {
                    visitedLocations.add(location);
                    orderedLocations.remove(location);

                    List<Direction> allowedDirections = ordersResults.get(orderId);

                    for (Direction direction : allowedDirections) {
                        Location nextLocation = direction.step(location);
                        explore(nextLocation);
                    }
                }
            }
        });

        thread.start();
    }

    private boolean isVisited(Location location) {
        return visitedLocations.contains(location);
    }

    private boolean isOrdered(Location location) {
        return orderedLocations.contains(location);
    }
}
