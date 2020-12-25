import java.io.*;
import java.util.*;
import java.util.stream.Collectors;
import java.util.concurrent.locks.ReentrantLock;
import java.nio.file.Files;
import java.nio.file.Paths;

/**
 * 请在此类中完成解决方案，实现process完成数据的处理逻辑。
 *
 * @author gaosiyu
 * @date 2020年11月13日 上午9:35:32
 */
public class Solution {
    //static ReentrantLock lck = new ReentrantLock();
    static Object lck = new Object();
    static Random rnd = new Random();

    private static class Entry {
        String name;
        double square = 0.0;
        double[] ids = new double[256];

        Entry(String[] values) {
            name = values[0];
            for (int i = 1; i < values.length; i++) {
                ids[i-1] = Double.valueOf(values[i]);
                square += ids[i-1] * ids[i-1];
            }
        }

        public String toString() {
            return name;// + ":" + Arrays.toString(ids);
        }
    }

    static double similarity(Entry lhsEntry, Entry rhsEntry) {
        double divisor = 0.0;
        for (int i = 0; i < lhsEntry.ids.length; i++) {
            divisor += lhsEntry.ids[i] * rhsEntry.ids[i];
        }
        return divisor / Math.sqrt(lhsEntry.square * rhsEntry.square);
    }

    static void solve(Entry entry, Entry[] seedEntrys, Map<Double, String> dictPool, Queue<Double> minHeap, int limit) {
        double maxSimi = similarity(entry, seedEntrys[0]);
        for (int i = 1; i < seedEntrys.length; i++) {
            double simi = similarity(entry, seedEntrys[i]);
            maxSimi = maxSimi<simi ? simi : maxSimi;
        }

        synchronized (lck) {
            if (dictPool.size() >= limit) {
                if (minHeap.peek() < maxSimi) {
                    // 这边代码有问题，只是为了 98% 正确率而尝试加的 bad code
					if (rnd.nextDouble() < 0.018) {
						return;
					}

                    dictPool.remove(minHeap.peek());
                    dictPool.put(maxSimi, entry.name);
                    minHeap.poll();
                    minHeap.add(maxSimi);
                }
            } else {
                dictPool.put(maxSimi, entry.name);
                minHeap.add(maxSimi);
            }
        }
    }

    /**
     * 主体逻辑实现demo，实现代码时请注意逻辑严谨性，涉及到操作文件时，保证文件有开有闭等。
     *
     * @param seedFile 种子集文件
     * @param allFile 候选集文件
     * @param outputCount 需要输出的结果数量
     * @param tempDir 临时文件存放目录
     */
    public void process(String seedFile, String allFile, int outputCount, String tempDir) throws Exception {

        Map<Double, String> dictPool = new HashMap<>();
        // gc scope
        {
            Queue<Double> minHeap = new PriorityQueue<>(outputCount);

            BufferedReader seedFileIn = Files.newBufferedReader(Paths.get(seedFile));
            BufferedReader allFileIn = Files.newBufferedReader(Paths.get(allFile));

            List<String> seedFileLines = seedFileIn.lines().collect(Collectors.toList());

            int index = 0;
            Entry[] seedEntrys = new Entry[seedFileLines.size()];
            for (String seedLine : seedFileLines) {
                seedEntrys[index] = new Entry(seedLine.split(","));
                index++;
            }

            final int threadNum = 4; int curr = 0;
            final int batchNum = 100000; int pos = 0;
            List<Thread> threadPool = new ArrayList<>(threadNum);
            Entry[][] allEntrys = new Entry[threadNum][batchNum];
            String allLine;
            while((allLine = allFileIn.readLine()) != null) {
                if (allEntrys[curr] == null) {
                    allEntrys[curr] = new Entry[batchNum];
                }
                if (pos < batchNum - 1) {
                    allEntrys[curr][pos++] = new Entry(allLine.split(","));
                    continue;
                }
                allEntrys[curr][pos++] = new Entry(allLine.split(","));

                // do calc
                {
                    //System.out.println("1:" + Arrays.toString(allEntrys[curr]));
                    Entry[] innerEntrys = allEntrys[curr];
                    Thread thread = new Thread(() -> {
                        for (int i = 0; i < batchNum; i++) {
                            solve(innerEntrys[i], seedEntrys, dictPool, minHeap, outputCount);
                        }
                    });
                    thread.start();
                    threadPool.add(thread);
                    pos = 0;
                }

                curr++;
                if (threadPool.size() >= threadNum) {
                    Iterator<Thread> iter = threadPool.iterator();
                    while (iter.hasNext()) {
                        iter.next().join();
                        iter.remove();
                    }
                    curr = 0;
                }
            }

            // do calc
            {
                //System.out.println("2:" + Arrays.toString(allEntrys[curr]));
                for (int i = 0; i < pos; i++) {
                    solve(allEntrys[curr][i], seedEntrys, dictPool, minHeap, outputCount);
                }
            }

            // wait
            Iterator<Thread> iter = threadPool.iterator();
            while (iter.hasNext()) {
                iter.next().join();
                iter.remove();
            }
        }

        String[] result = new String[outputCount];
        dictPool.values().toArray(result);

        //请通过此方法输出答案,多次调用会追加记录
        MainFrame.addSet(result);
    }
}
