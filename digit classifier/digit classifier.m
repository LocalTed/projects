clc; clear;

%% ====================== Prepare New Test Set ======================
load('mnist.mat');

num_images = test.count;
new_test_images = shiftdim(test.images, 2);
A_new_test = reshape(new_test_images,num_images,28*28);
A_new_test = [A_new_test, ones(num_images,1)];
true_labels = test.labels;

%% ============================ Predict ==============================
%% training 10 recognizers
N = 4000;
classifier = zeros((28^2)+1, 10);

for k=0:9
    imagesPerDigit = training.images(:,:,training.labels == k);
    imagesPerOthers = training.images(:,:,training.labels ~= k);

    A_all = zeros(2*N,28^2);
    b_all = zeros(2*N,1);

    for i=1:N
        A_all(i,:) = reshape(imagesPerDigit(:,:,i),1,28^2);
        b_all(i)   = +1;
    end
    for i=N+1:2*N
        A_all(i,:) = reshape(imagesPerOthers(:,:,i),1,28^2);
        b_all(i)   = -1;
    end

    classifier(:,k+1)=pinv([A_all, ones(2*N,1)])*b_all;
end

%% predicting
pred = zeros(size(true_labels));
for i=1:size(true_labels)
    result = A_new_test(i,:)*classifier;
    result = abs(1-result);
    pred(i) = find(result==min(result))-1;
end

%% =========================== Evaluate ==============================
acc = mean(pred == true_labels)*100;
disp(['Accuracy=',num2str(acc),'% (',num2str((1-acc/100)*num_images),' wrong examples)']); 

error = find(pred~=true_labels); 
for k=1:1:5
    figure(2);
    imagesc(reshape(A_new_test(error(k),1:28^2),[28,28]));
    colormap(gray(256))
    axis image; axis off; 
    title(['problematic digit number ',num2str(k)]); 
    pause;  
end

