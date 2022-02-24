import map.MapTask;
import map.Results;
import reduce.ReduceResult;
import reduce.ReduceTask;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.concurrent.Semaphore;

public class Tema2 {
    // size of one fragment
    static int D;

    // no. documents
    static int N;

    // documents' path/names
    static List<String> docs;
    static List<String> docNames;

    // no. workers/threads
    static int P;

    static List<MapTask> mapTasks;
    static int mapId;

    // the results given by the Map tasks
    // <document_name, list of results>
    static Results results;

    static List<ReduceTask> reduceTasks;

    static Map<String, Semaphore> semaphoreMap;

    static boolean checkAlphaNumeric(int b) {
        return (b >= 'a' && b <= 'z') ||
               (b >= 'A' && b <= 'Z') ||
               (b >= '0' && b <= '9');
    }

    /**
     * Generate the tasks for a document
     * @param doc - the document to be divided (path)
     * @param docName - name of the document
     */
    static void genDocumentTasks(String doc, String docName) throws IOException {
        // current offset
        int off = 0;

        var size = Files.size(Path.of(doc));

        var ct = size / D;
        // check for remaining bytes
        var r = size % D;

        var sem_val = ct;
        if (r > 0) {
            ++sem_val;
        }
        Semaphore sem = new Semaphore((int) (-sem_val + 1));
        semaphoreMap.put(docName, sem);

        RandomAccessFile raf = new RandomAccessFile(doc, "r");

        int i;
        long taskD, prev = 0;
        for (i = 0; i < ct; ++i) {
            // subtract the length of the word split
            // (if any)
            taskD = D - prev;
            prev = 0;

            // check if the current task would
            // be ending inside a word
            raf.seek(off + taskD - 1);

            // check whether there are
            // 2 alphanumeric characters
            //  > one at the end of the current task
            int bLast = raf.read();
            //  > one at the beginning of the next task
            int bFirst = raf.read();

            if (checkAlphaNumeric(bLast) && checkAlphaNumeric(bFirst)) {
                // read a line from the next task
                // get the first word
                //  --> should be included in the current task
                raf.seek(off + taskD);
                String l = raf.readLine();
                String[] aux = l.split("[^\\p{Alnum}]+");

                prev = aux[0].length();
                taskD += aux[0].length();
            }

            ++mapId;
            MapTask mt = new MapTask(docName, off, taskD, raf, mapId, results, sem);
            mapTasks.add(mt);

            off += taskD;
        }

        if (r > 0) {
            ++mapId;
            taskD = r - prev;
            MapTask mt = new MapTask(docName, off, taskD, raf, mapId, results, sem);
            mapTasks.add(mt);
        }
    }

    // generate the Map operation tasks
    static void genMapTasks() throws IOException {
        int i;
        for (i = 0; i < docs.size(); ++i) {
            genDocumentTasks(docs.get(i), docNames.get(i));
        }
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }

        // initialise
        docs = new ArrayList<>();
        docNames = new ArrayList<>();

        mapTasks = new ArrayList<>();
        mapId = -1;

        reduceTasks = new ArrayList<>();

        // parse the arguments
        P = Integer.parseInt(args[0]);
        String in = args[1];
        String out = args[2];

        BufferedReader fin;
        fin = new BufferedReader(new FileReader(in));

        String str;

        str = fin.readLine();
        D = Integer.parseInt(str);

        str = fin.readLine();
        N = Integer.parseInt(str);

        int i;

        String[] aux;
        List<String> arr;
        for (i = 0; i < N; ++i) {
            str = fin.readLine();

            aux = str.split("/");
            arr = new ArrayList<>(Arrays.asList(aux));
            // extract the name of the document
            docNames.add(arr.get(arr.size() - 1));

            docs.add(str);
        }

        results = new Results(docNames);
        semaphoreMap = new HashMap<>();

        // generate the Map tasks
        genMapTasks();
        int numMapTasks = mapTasks.size();

        // initialise the threads;
        MRThread[] threads = new MRThread[P];
        for (i = 0; i < P; ++i) {
            threads[i] = new MRThread();
        }

        // allocate the tasks to the threads
        int r;
        // MAP
        for (i = 0; i < numMapTasks; ++i) {
            r = i % P;

            threads[r].addMapTask(mapTasks.get(i));
        }

        // REDUCE
        var docResults = results.docResults;
        for (i = 0; i < N; ++i) {
            r = i % P;

            String name = docNames.get(i);
            ReduceTask t = new ReduceTask(name, docResults.get(name), i, semaphoreMap.get(name));
            reduceTasks.add(t);

            threads[r].addReduceTask(t);
        }

        for (i = 0; i < P; ++i) {
            threads[i].start();
        }

        for (i = 0; i < P; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Parse the results
        List<ReduceResult> docRanks = new ArrayList<>();
        for (var t : reduceTasks) {
            docRanks.add(t.getResult());
        }

        docRanks.sort(new Comparator<ReduceResult>() {
            @Override
            public int compare(ReduceResult a, ReduceResult b) {
                if (a.getRank() == b.getRank()) {
                    return (a.getId() - b.getId());
                }

                double r = b.getRank() - a.getRank();
                if (r > 0) {
                    return 1;
                }
                return -1;
            }
        });

        BufferedWriter fout = new BufferedWriter(new FileWriter(out));
        for (var d : docRanks) {
            fout.append(d.getName()).append(",")
                    .append(String.format("%.2f", d.getRank())).append(",")
                    .append(String.valueOf(d.getMaxLen())).append(",")
                    .append(String.valueOf(d.getOcc())).append("\n");
        }
        fout.close();
    }
}
