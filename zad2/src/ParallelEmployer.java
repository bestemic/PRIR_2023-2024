import java.util.List;

public class ParallelEmployer implements Employer {

    private OrderInterface orderInterface;

    @Override
    public void setOrderInterface(OrderInterface order) {
        orderInterface = order;
        orderInterface.setResultListener(result -> {

        });
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        return null;
    }
}
