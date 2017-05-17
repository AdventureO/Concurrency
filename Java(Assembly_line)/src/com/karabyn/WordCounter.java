package com.karabyn;

/**
 * Created by petro on 18-Apr-17.
 */
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.concurrent.*;

public class WordCounter implements Runnable {

    private BlockingQueue<String> blocksQueue;
    private BlockingQueue<HashMap> mergeQueue;
    private static int numberOfThreads = 0;

    public WordCounter(BlockingQueue<String> blocksQueue, BlockingQueue<HashMap> mergeQueue) {
        this.blocksQueue = blocksQueue;
        this.mergeQueue = mergeQueue;
        numberOfThreads += 1;
    }

    @Override
    public void run() {
        long countingStartTime = System.nanoTime(); // Detect time when WordCounter threads start running
        HashMap<String,Integer> wordCountHashMap = new HashMap<>();
        String text = "";
        while(true) {
            try {
                text = blocksQueue.take();
                // CASE 1: One active WordCounted thread left. Receives a poison pill, sends a new poison pill to Merger
                // to notify that WordCounter finished working. Terminates itself.
                if(text.equals("POISONPILL") && numberOfThreads == 1) {
                    wordCountHashMap.put("POISONPILL", 0);
                    mergeQueue.put(wordCountHashMap);
                    // Calculate total counting time when the last thread finishes work.
                    long countingExecutionTime = System.nanoTime() - countingStartTime;
                    try {
                        FileWriter fw = new FileWriter("outputFile.txt", true);
                        fw.append("Counting time: ").append(timeToString(countingExecutionTime)).append("\n");
                        fw.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    System.out.println("Counting time: " + timeToString(countingExecutionTime));
                    break;
                }
                // CASE 2: Multiple active WordCounter threads. One of them receives a poison pill(terminating condition),
                // sends it to other active WordCounter threads and terminates itself.
                else if(text.equals("POISONPILL")) {
                    blocksQueue.put("POISONPILL");
                    numberOfThreads -= 1;
                    break;
                }
                // CASE 3: Multiple active WordCounter threads. Poison pill not received yet. Continue work.
                else {
                    String[] block = extractOnlyWords(text);
                    wordCountHashMap = wordCount(block);
                    mergeQueue.put(wordCountHashMap);
                    wordCountHashMap = new HashMap<>();
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private String[] extractOnlyWords(String text) {
        return text.replaceAll("\\W|\\d|_", " ").toLowerCase().split("\\s++");
    }

    private HashMap<String, Integer> wordCount(String[] words) {
        HashMap<String, Integer> wordCountHashMap = new HashMap<>();
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

    private String timeToString(long time) {
        String timeString = String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
        return timeString;
    }
}