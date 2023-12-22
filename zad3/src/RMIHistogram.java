import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashMap;
import java.util.Map;

public class RMIHistogram extends UnicastRemoteObject implements RemoteHistogram, Binder {

    private int id;
    private final Map<Integer, int[]> histogramsMap;

    public RMIHistogram() throws RemoteException {
        id = 0;
        histogramsMap = new HashMap<>();
    }

    @Override
    public void bind(String serviceName) {
        try {
            Registry registry = LocateRegistry.getRegistry("localhost", 1099);
            registry.rebind(serviceName, this);
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public synchronized int createHistogram(int bins) throws RemoteException {
        histogramsMap.put(++id, new int[bins]);
        return id;
    }

    @Override
    public synchronized void addToHistogram(int histogramID, int value) throws RemoteException {
        histogramsMap.get(histogramID)[value]++;
    }

    @Override
    public synchronized int[] getHistogram(int histogramID) throws RemoteException {
        return histogramsMap.get(histogramID);
    }
}
