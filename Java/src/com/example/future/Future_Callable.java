package com.example.future;

/**
 * Created by petro on 16-Jun-17.
 */
import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class Future_Callable {

    private static StringBuilder sb = new StringBuilder();
    private static final File runconfig = new File("runconfig.txt");
    private static File filename; //= new File("src/books1-14.txt"); file to read text from
    private static int numberOfThreads;
    private static String[] textBlocks;
    private static HashMap<String, Integer> mainHashMap = new HashMap<>();

    public static void main(String[] args) {
        Future_Callable wordCounter = new Future_Callable();
        wordCounter.getRunConfig();
        textBlocks = new String[numberOfThreads];

        System.out.printf("Java Future\nFile: %s\nThreads: %s\n", filename.toString(), numberOfThreads);

        // reading
        long readingStartTime = System.nanoTime();
        wordCounter.readFile(filename);
        long readingExecutionTime = System.nanoTime() - readingStartTime;
        //System.out.println("Reading time: " + wordCounter.timeToString(readingExecutionTime));
        //System.out.println("Reading time: " + wordCounter.timeToStringMicro(readingExecutionTime));

        // extracting and counting words in threads
        long wordCountStartTime = System.nanoTime();
        wordCounter.divideString();

        long calculatingStartTime = System.nanoTime();
        ExecutorService threadpool = Executors.newFixedThreadPool(numberOfThreads); // create a pool of threads
        List <Future<HashMap<String, Integer>>> list = new ArrayList<Future<HashMap<String, Integer>>>();
        for (int i = 0; i < numberOfThreads; i++) {
            Future<HashMap<String, Integer>> future = threadpool.submit(new CounterThread(textBlocks[i]));
            list.add(future);
        }

        HashMap<String, Integer> hashMap = new HashMap<>();
        for(java.util.concurrent.Future<HashMap<String, Integer>> fut : list) {
            try {
                hashMap = fut.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
            for (Map.Entry<String, Integer> e : hashMap.entrySet())
                mainHashMap.merge(e.getKey(), e.getValue(), Integer::sum);
        }
        threadpool.shutdown();

        long calculatingExecutionTime = System.nanoTime() - calculatingStartTime;
        System.out.println("Calculating time: " + wordCounter.timeToStringMicro(calculatingExecutionTime));
        long wordCountExecutionTime = System.nanoTime() - wordCountStartTime;
        //System.out.println("Word count time: " + wordCounter.timeToString(wordCountExecutionTime));
        //System.out.println("Word count time: " + wordCounter.timeToStringMicro(wordCountExecutionTime));

        long totalexecutionTime = readingExecutionTime + wordCountExecutionTime;
        //System.out.println("Total time: " + wordCounter.timeToString(totalexecutionTime));
        System.out.println("Total time: " + wordCounter.timeToStringMicro(totalexecutionTime));

        //wordCounter.sortByOccurrences(mainHashMap);
        //wordCounter.sortByAlphabet(mainHashMap);
        //wordCounter.writeOutputFile(readingExecutionTime, wordCountExecutionTime, totalexecutionTime);

    }

    private void getRunConfig() {
        try {
            BufferedReader bufferedReader = new BufferedReader(new FileReader(runconfig));
            String input = bufferedReader.readLine();
            numberOfThreads = Integer.valueOf(input.substring(0, input.indexOf(" ")));
            input = bufferedReader.readLine();
            filename = new File(input.substring(0, input.indexOf(" ")));

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void readFile(File filename) {
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = new BufferedReader(new FileReader(filename));
            String line = bufferedReader.readLine();
            while (line != null) {
                sb.append(line);
                sb.append("\n");
                line = bufferedReader.readLine();
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                bufferedReader.close();
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Divides the input text into equal parts according
     * to the number of threads for further processing.
     */
    private void divideString() {
        String text = sb.toString();
        int textLength = text.length();
        double n = numberOfThreads;
        for(int i = 0; i < numberOfThreads; i ++) {
            textBlocks[i] = text.substring((int)(textLength * (i / n)), (int)(textLength * ((i + 1) / n)));
        }

    }

    private void sortByOccurrences(HashMap wordCountHashMap) {
        List list = new ArrayList(wordCountHashMap.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {
            @Override
            public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
                return b.getValue() - a.getValue();
            }
        });
        writeToFile(list, "wordsByOccurrencesOutput.txt");
        System.out.println("wordsByOccurrences: " + list);
    }

    private void sortByAlphabet(HashMap wordCountHashMap) {
        List list = new ArrayList(wordCountHashMap.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {
            @Override
            public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
                return a.getKey().compareTo(b.getKey());
            }
        });
        writeToFile(list, "wordsByAlphabet.txt");
    }

    private void writeToFile(List list, String filename) {
        try{
            PrintWriter writer = new PrintWriter(filename, "UTF-8");
            for(Object entry : list) {
                HashMap.Entry e = (HashMap.Entry) entry;
                writer.println(e.getKey() + "=" + e.getValue());
            }
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void writeOutputFile(long reading, long count, long total) {
        try{
            PrintWriter writer = new PrintWriter("outputFile.txt", "UTF-8");
            writer.println("Reading time: " + timeToString(reading));
            writer.println("Word count time: " + timeToString(count));
            writer.println("Total time: " + timeToString(total));
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String timeToString(long time) {
        return String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
    }

    private String timeToStringMicro(long time) {
        return String.format("%d microseconds", TimeUnit.NANOSECONDS.toMicros(time));
    }

    private int numberOfDistinctWords(HashMap<String, Integer> wordCount) {
        return wordCount.size();
    }

}
class CounterThread implements Callable<HashMap<String, Integer>> {

    private String text;
    private HashMap<String,Integer> wordCountHashMap = new HashMap<>();

    CounterThread(String text) {
        this.text=text;
    }

    private String[] extractOnlyWords() {
        return text.replaceAll("\\W|\\d|\\_", " ").toLowerCase().split("\\s++");
    }

    private HashMap<String, Integer> wordCount(String[] words) {
        for(String word : words) {
            if(!wordCountHashMap.containsKey(word)){
                wordCountHashMap.put(word, 1);
            }
            else {
                wordCountHashMap.put(word, wordCountHashMap.get(word) + 1);
            }
        }
        return wordCountHashMap;
    }

    @Override
    public HashMap<String, Integer> call() throws Exception {

        String[] words = extractOnlyWords();
        for(String word : words) {
            if(!wordCountHashMap.containsKey(word)){
                wordCountHashMap.put(word, 1);
            }
            else {
                wordCountHashMap.put(word, wordCountHashMap.get(word) + 1);
            }
        }
        return wordCountHashMap;
    }
}


