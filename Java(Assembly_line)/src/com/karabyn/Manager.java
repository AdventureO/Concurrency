package com.karabyn;

import java.io.*;
import java.util.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

public class Manager {

    public static void main(String[] args) {
        // !!! Please, enter a correct path to the configuration file(runconfig.txt) and
        // to the analysis file(book1-14.txt) in the configuration file before running the program.
        String runconfigFilePath = "C:\\Users\\petro\\IdeaProjects\\Java(Assembly_line)\\runconfig.txt";

        long executionStartTime = System.nanoTime(); // Detect time when the program started

        // Getting configurations
        int numberOfThreads = 0;
        int blocksize = 0;
        File filename = new File("");
        try {
            BufferedReader bufferedReader = new BufferedReader(new FileReader(new File(runconfigFilePath)));
            String input = bufferedReader.readLine();
            numberOfThreads = Integer.valueOf(input.substring(0, input.indexOf(" ")));
            input = bufferedReader.readLine();
            blocksize = Integer.valueOf(input.substring(0, input.indexOf(" ")));
            input = bufferedReader.readLine();
            filename = new File(input.substring(0, input.indexOf(" ")));
        } catch (IOException e) {
            e.printStackTrace();
        }
        //Creating shared blocksQueue and mergeQueue
        BlockingQueue<String> blocksQueue = new LinkedBlockingQueue<>();
        BlockingQueue<HashMap> mergeQueue = new LinkedBlockingQueue<>();

        //Creating Producer and WordCounter and Merger threads
        Thread blockReaderThread = new Thread(new BlockReader(blocksQueue, blocksize, filename));
        Thread[] wordCounterThreadsArray = new Thread[numberOfThreads];
        for(int i = 0; i < numberOfThreads; i++) {
            wordCounterThreadsArray[i] = new Thread(new WordCounter(blocksQueue, mergeQueue));
        }
        Thread[] mergerThreadsArray = new Thread[numberOfThreads];
        for(int i = 0; i < numberOfThreads; i++) {
            mergerThreadsArray[i] = new Thread(new Merger(mergeQueue));
        }

        //Starting BlockReader, WordCounter and Merger threads
        blockReaderThread.start();
        for(Thread wordCounterThread : wordCounterThreadsArray){
            wordCounterThread.start();
        }
        for(Thread mergerThread : mergerThreadsArray) {
            mergerThread.start();
        }

       for(Thread mergerThread : mergerThreadsArray) {
            try {
                mergerThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        sortByOccurrences(mergeQueue.peek());
        sortByAlphabet(mergeQueue.peek());

        long executionTotalTime = System.nanoTime() - executionStartTime;
        try {
            FileWriter fw = new FileWriter("outputFile.txt", true);
            fw.append("Total execution time: ").append(timeToString(executionTotalTime)).append("\n");
            fw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("Total execution time: " + timeToString(executionTotalTime));

    }

    private static void sortByOccurrences(HashMap hashMap) {
        List list = new ArrayList(hashMap.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {
            @Override
            public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
                return b.getValue() - a.getValue();
            }
        });
        writeToFile(list, "wordsByOccurrencesOutput.txt");
        System.out.println("wordsByOccurrences: " + list);
    }

    private static void sortByAlphabet(HashMap wordCountHashMap) {
        List list = new ArrayList(wordCountHashMap.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {
            @Override
            public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
                return a.getKey().compareTo(b.getKey());
            }
        });
        writeToFile(list, "wordsByAlphabetOutput.txt");
    }

    private static void writeToFile(List list, String filename) {
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

    private static String timeToString(long time) {
        String timeString = String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
        return timeString;
    }
}