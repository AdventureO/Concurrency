package com.karabyn;

import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;


/**
 * Created by petro on 18-Apr-17.
 */

public class Merger implements Runnable {

    private BlockingQueue<HashMap> mergeQueue;
    private static int numberOfThreads = 0;

    public Merger(BlockingQueue<HashMap> mergeQueue) {
        this.mergeQueue = mergeQueue;
        numberOfThreads += 1;
    }

    @Override
    public void run() {
        long mergingStartTime = System.nanoTime(); // Detect time when Merger threads start running
        HashMap<String, Integer> emptyHashMap = new HashMap<>();
        while(true){
            try {
                HashMap<String, Integer> map1 = mergeQueue.take();
                if(mergeQueue.isEmpty()){ // Prevent thread from infinitely waiting for take() to execute when queue is empty
                    mergeQueue.put(emptyHashMap);
                }
                HashMap<String, Integer> map2 = mergeQueue.take();
                HashMap<String, Integer> map3 = new HashMap<>(map1);
                for (Map.Entry<String, Integer> e : map2.entrySet()) {
                    map3.merge(e.getKey(), e.getValue(), Integer::sum);
                }
                mergeQueue.put(map3);
                if(mergeQueue.size() == 1 && mergeQueue.peek().containsKey("POISONPILL")){
                    numberOfThreads -= 1;
                    break;
                }
            }
            catch (InterruptedException | NullPointerException e) {
                // Expected exception handled
            }
        }
        if (numberOfThreads == 0) {
            // Calculate total counting time when the last thread finishes work.
            long mergingExecutionTime = System.nanoTime() - mergingStartTime;
            try {
                FileWriter fw = new FileWriter("outputFile.txt", true);
                fw.append("Merging time: ").append(timeToString(mergingExecutionTime)).append("\n");
                fw.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println("Merging time: " + timeToString(mergingExecutionTime));
        }
    }

    private String timeToString(long time) {
        String timeString = String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
        return timeString;
    }
}
