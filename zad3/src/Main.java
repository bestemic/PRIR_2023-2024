import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.util.Arrays;

public class Main {
    public static void main(String[] args) throws RemoteException {

        int PORT = 1099;
        LocateRegistry.createRegistry(PORT);

        RMIHistogram rmiHistogram = new RMIHistogram();
        rmiHistogram.bind("RMI_TESTER");
    }
}