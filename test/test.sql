-- create table
-- 没有指定主键的
create table Student (
	sno char(12)
);
-- 重复定义属性的
create table Student (
	sno char(12) primary key,
	sno int
);
-- 语义正确的
create table Student (
	Sno char(12) primary key,
	Sname char(9) unique,
	Sex char(3),
	Height double PRECISION not null,
	Dept char(12) 
);
create table Student (
	Sno char(12) primary key,
	Sname char(9) unique,
	Sex char(3),
	Height real not null,
	Dept char(12) 
);
create table Student ( -- 这个时候再创建就报错已存在 Student表
	Sno char(12) primary key
);

-- drop table 
drop table Student;
drop table Student;	-- 第二次drop就报错不存在 Student表

-- insert
-- 不存在目标表的
insert into Teacher values('L', 55, '男');
-- 属性数量不匹配的
insert into Student values('L');
insert into Student(Sname) values(4, 5);
-- 主属性/not-null属性为空
insert into Student values(null, '廖培湧', '男', 1.75, '计算机系');
insert into Student values('201630471516', 'A23187', '男', null, '计算机系');
insert into Student(Sno, Height) values(null, 1.75);
insert into Student(Sno, Height)m values('201630471516', null);
-- 类型不能一一匹配的
insert into Student values(123, '廖培湧', '男', 1.75, '计算机系');
insert into Student values('201630471516', '廖培湧', '男', 2, '计算机系');
insert into Student(Sno, Height) values(123, 1.7);
insert into Student(Sno, Height) values('201630471516', 1);
-- 字符串长度太大的
insert into Student values('201630471516', 'Leo" Messi', '男', 1.75, '计算机系');
insert into Student(Sno, Height) values('201630471516201630471516', 1.75);
-- 瞎写属性的
insert into Student(Age) values(20);
-- 指定的属性没有包含所有主属性/not-null属性的
insert into Student(Sname) values('A23187');
-- 语义正确的
insert into Student values('201630471516', '廖培湧', '男', 1.75, '计算机系');
insert into Student values('111111111111', 'A23187', null, 1.8, 'CS');
insert into Student(Sno, Sname, Height, Sex) values('201630471516', 'A23187', 1.75, null);

-- 插入一些数据以测试
insert into Student values('201530311042', '陈浩鑫', '女', 1.69, '计算机系');
insert into Student values('201530541661', '李雨键', '男', 1.72, '计算机系');
insert into Student values('201530541838', '吕扬', '男',  1.81, '计算机系');
insert into Student values('201530542156', '吴伟斌', '男', 1.80, '计算机系');
insert into Student values('201530542262', '杨晓虹', '男', 1.76, '计算机系');

-- select
-- 不存在目标表的
select * from Teacher;
-- select *, xxx from
select *, Sno from Student;
-- select xx, * from
select Sno, * from Student;
-- 瞎写属性的
select count(Age) from Student;
select Age from Student;
-- 不能在where子句中使用聚集函数
select * from Student where count(*) > 10;
-- 在where子句中瞎写属性
select * from Student where Age < 20;
-- 语义正确的
select * from Student;
select Sno, Sname from Student;
select Sno from Student where Sno = '201530371190'; -- 不存在的
select Sno, Sname, Sex from Student where Sno = '201530'+'311042';
select Sno, Height from Student where Height < 1.72;
select Sno, Height from Student where Height between 1.60 and 1.72;


-- update
-- 不存在目标表的
update Teacher set Tno='007';
-- 瞎写属性的
update Student set Age=20;
-- 将主属性/not-null属性更新为空
update Student set Sno=null;
update Student set Height=null;
-- 类型不能一一匹配的
update Student set Sno=1.5;
update Student set Height='...';
update Student set Height=2;
-- 不能在where子句中使用聚集函数
update Student set Height=1.7 where count(*) > 10;
-- 在where子句中瞎写属性
update Student set Height=1.7 where Age < 20;
-- 语义正确的
update Student set Height=1.78;
update Student set Height=1.5, Sex=null;
update Student set Sex='男' where Sex is null;
update Student set Sex=null where Sex='男';


-- delete
-- 不存在目标表的
delete from Teacher;
-- 不能在where子句中使用聚集函数
delete from Student where count(*) > 10; --
-- 在where子句中瞎写属性
delete from Student where Age < 20; --
-- 语义正确的
delete from Student;
delete from Student where Sno='201530541838';


-- 一大波数据
insert into Student values('201530471586', '邱伟峰', '男', 1.70, '计算机系');
insert into Student values('201536541429', '胡顺宏', '女', 1.72, '计算机系');
insert into Student values('201536061248', '柯皓文', '男', 1.85, '计算机系');
insert into Student values('201530541364', '何杏萍', '女', 1.64, '计算机系');
insert into Student values('201530541746', '林世畅', '男', 1.79, '计算机系');
insert into Student values('201530541128', '陈若晖', '男', 1.70, '计算机系');
insert into Student values('201530542620', '郑梓悫', '女', 1.63, '计算机系');
insert into Student values('201530541067', '陈嘉健', '男', 1.68, '计算机系');
insert into Student values('201530541111', '陈润森', '男', 1.69, '计算机系');
insert into Student values('201530541487', '黄喜全', '男', 1.73, '计算机系');
insert into Student values('201530541678', '李智豪', '女', 1.66, '计算机系');
insert into Student values('201530542521', '赵菲苑', '男', 1.78, '计算机系');
insert into Student values('201530541371', '何亿', '女', 1.58, '计算机系');
insert into Student values('201530541098', '陈可可', '男', 1.68, '计算机系');
insert into Student values('201530541418', '胡景铨', '男', 1.73, '计算机系');
insert into Student values('201530541210', '邓勇达', '男', 1.81, '计算机系');
insert into Student values('201530542545', '赵伟杰', '女', 1.59, '计算机系');
insert into Student values('201530541654', '李心妍', '女', 1.67, '计算机系');
insert into Student values('201530541043', '陈海康', '男', 1.75, '计算机系');
insert into Student values('201592450584', '黄琳', '男', 1.82, '计算机系');
insert into Student values('201530541807', '刘俊彦', '男', 1.68, '计算机系');
insert into Student values('201530542484', '张智杰', '女', 1.63, '计算机系');
insert into Student values('201530542613', '郑晓燕', '男', 1.80, '计算机系');
insert into Student values('201530542415', '张朝晖', '男', 1.70, '计算机系');
insert into Student values('201530542286', '杨心语', '男', 1.80, '计算机系');
insert into Student values('201530541159', '陈钊浩', '女', 1.60, '计算机系');
insert into Student values('201536542334', '应澄粲', '男', 1.71, '计算机系');
insert into Student values('201530541524', '靳泽宇', '男', 1.76, '计算机系');
insert into Student values('201530541340', '何浩宁', '男', 1.85, '计算机系');
insert into Student values('201530541692', '梁尉鑫', '男', 1.77, '计算机系');
insert into Student values('201530541234', '范晓桐', '男', 1.71, '电机系');
insert into Student values('201530542446', '张睿', '男', 1.84, '电机系');
insert into Student values('201530541913', '邱子昀', '男', 1.70, '电机系');
insert into Student values('201530661079', '冯嘉昌', '女', 1.59, '电机系');
insert into Student values('201530381885', '徐映雪', '男', 1.80, '电机系');
insert into Student values('201592450587', '刘亚威', '女', 1.65, '电机系');
insert into Student values('201530542347', '游德鸿', '女', 1.68, '电机系');
insert into Student values('201530541357', '何伟健', '女', 1.60, '电机系');
insert into Student values('201530541302', '古超文', '女', 1.56, '电机系');
insert into Student values('201530541258', '封含儒', '男', 1.74, '电机系');
insert into Student values('201530541975', '孙越', '女', 1.69, '电机系');
insert into Student values('201530542187', '肖振鹏', '男', 1.70, '电机系');
insert into Student values('201530542033', '王锦杰', '女', 1.70, '电机系');
insert into Student values('201530541944', '宋林涵', '男', 1.69, '电机系');
insert into Student values('201530541845', '罗睿琪', '男', 1.83, '电机系');
insert into Student values('201530541685', '梁浩赞', '女', 1.64, '电机系');
insert into Student values('201530542149', '吴松树', '女', 1.58, '电机系');
insert into Student values('201530542453', '张熙程', '男', 1.81, '电机系');
insert into Student values('201530541937', '沈强', '男', 1.78, '电机系');
insert into Student values('201530541876', '麦梓旗', '男', 1.69, '电机系');
insert into Student values('201530541166', '陈庄智', '女', 1.71, '电机系');
insert into Student values('201530541296', '龚壮邦', '男', 1.71, '电机系');
insert into Student values('201530541463', '黄明', '男', 1.69, '电机系');
insert into Student values('201530541326', '郭昊岚', '男', 1.80, '电机系');
insert into Student values('201535542519', '赵飞扬', '女', 1.60, '电机系');
insert into Student values('201530541333', '郭允慈', '男', 1.80, '电机系');
insert into Student values('201530541456', '黄凯博', '男', 1.70, '电机系');
insert into Student values('201530541401', '洪浩强', '男', 1.84, '电机系');
insert into Student values('201530542378', '曾繁忠', '男', 1.70, '电机系');
insert into Student values('201530371763', '郑佳炜', '男', 1.85, '电机系');
insert into Student values('201530542576', '郑春玲', '男', 1.81, '电机系');
insert into Student values('201592450588', '张嘉奥', '男', 1.72, '电机系');
insert into Student values('201530542293', '杨奕彬', '女', 1.65, '电机系');
insert into Student values('201530542224', '许树雄', '男', 1.84, '电机系');
insert into Student values('201530542002', '唐仁杰', '女', 1.72, '电机系');
insert into Student values('201592450586', '李研', '女', 1.57, '电机系');
insert into Student values('201530541388', '何盈盈', '女', 1.60, '电机系');
insert into Student values('201530542231', '许哲', '女', 1.58, '电机系');
insert into Student values('201530541623', '李君豪', '男', 1.78, '电机系');
insert into Student values('201530371190', '范港平', '女', 1.60, '电机系')
