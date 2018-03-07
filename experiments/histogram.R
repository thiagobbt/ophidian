library(stringr)
library(RColorBrewer)

args = commandArgs(TRUE)
path = args[1]
raw_data = read.csv(path);
names(raw_data) = c("displacement")
data = as.numeric(str_sub(raw_data$displacement, 1, str_length(raw_data$displacement)-3))

circuitName = tools::file_path_sans_ext(basename(path))

maxDisplacement = max(data)
breaks = seq(from=0, to=maxDisplacement+250, by=250)
colors = brewer.pal(n=12, name="Set3")

plotFile = paste(circuitName, "pdf", sep=".")

# pdf(plotFile)

# hist(data, col=colors, xaxt="n", xlim=c(40000, 0), xlab="Displacement", ylim=c(0, 500), breaks=breaks)
# axis(1, at=seq(from=40000, to=0, by=-500), las=2)

# dev.off()

# message(paste("File saved to", plotFile))
message(paste(path))
message("max: ", max(data))
message("mean: ", mean(data))
message("sum: ", sum(data))

